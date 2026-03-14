#include "Cyclops/Scene/Canvas.h"
#include "Cyclops/Platform/OpenGL/OpenGLDebug.h"
#include "Cyclops/Renderer/Renderer2D.h"

#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>

namespace Cyclops
{
    Canvas::Canvas(uint32_t width, uint32_t height)
        : m_Width(width), m_Height(height)
    {
        FramebufferSpecification spec;
        spec.Width = width;
        spec.Height = height;
        spec.Formats = { FramebufferTextureFormat::RGBA8 };
        m_CompositeBuffer = Framebuffer::Create(spec);

        // Create Default Layer
        AddLayer("Background");

        // Clear Background (White)
        auto fbo = m_Layers[0].GetActiveFramebuffer(0);
        if (fbo)
        {
            fbo->Bind();
            GLCall(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
            GLCall(glClear(GL_COLOR_BUFFER_BIT));
            fbo->Unbind();
        }

        Recompose();
    }

    void Canvas::AddLayer(const std::string& name)
    {
        m_Layers.emplace_back(m_Width, m_Height, name);
        m_ActiveLayerIndex = (int)m_Layers.size() - 1;

        // Clear the new layer's first keyframe
        auto fbo = m_Layers.back().GetActiveFramebuffer(m_CurrentFrame);
        if (fbo)
        {
            fbo->Bind();
            GLCall(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
            GLCall(glClear(GL_COLOR_BUFFER_BIT));
            fbo->Unbind();
        }

        Recompose();
    }

    void Canvas::RemoveLayer(int index)
    {
        if (m_Layers.size() <= 1) return; // Don't delete the last layer

        if (index >= 0 && index < m_Layers.size())
        {
            // If we are deleting the active layer, default to 0
            if (m_ActiveLayerIndex == index)
                m_ActiveLayerIndex = 0;
            // If we delete a layer *below* active, shift active index down
            else if (index < m_ActiveLayerIndex)
                m_ActiveLayerIndex--;

            m_Layers.erase(m_Layers.begin() + index);

            // Safety clamp
            if (m_ActiveLayerIndex >= m_Layers.size())
                m_ActiveLayerIndex = (int)m_Layers.size() - 1;

            Recompose();
        }
    }

    void Canvas::InsertLayer(int index, const CanvasLayer& layer)
    {
        if (index < 0 || index > m_Layers.size()) index = (int)m_Layers.size();

        m_Layers.insert(m_Layers.begin() + index, layer);

        // Fix active index
        if (index <= m_ActiveLayerIndex) m_ActiveLayerIndex++;

        Recompose();
    }

    void Canvas::MoveLayer(int fromIndex, int toIndex)
    {
        if (fromIndex == toIndex) return;
        if (fromIndex < 0 || fromIndex >= m_Layers.size()) return;
        if (toIndex < 0 || toIndex >= m_Layers.size()) return;

        // 1. Copy
        CanvasLayer layer = m_Layers[fromIndex];
        // 2. Remove
        m_Layers.erase(m_Layers.begin() + fromIndex);
        // 3. Insert
        m_Layers.insert(m_Layers.begin() + toIndex, layer);

        // 4. Update Selection
        if (m_ActiveLayerIndex == fromIndex)
        {
            m_ActiveLayerIndex = toIndex;
        }
        else if (fromIndex < m_ActiveLayerIndex && toIndex >= m_ActiveLayerIndex)
        {
            m_ActiveLayerIndex--;
        }
        else if (fromIndex > m_ActiveLayerIndex && toIndex <= m_ActiveLayerIndex)
        {
            m_ActiveLayerIndex++;
        }
    }

    void Canvas::SetActiveLayer(int index)
    {
        if (index >= 0 && index < m_Layers.size())
        {
            m_ActiveLayerIndex = index;
        }
    }

    void Canvas::SetCurrentFrame(int frameIndex)
    {
        if (frameIndex < 0) frameIndex = 0;
        m_CurrentFrame = frameIndex;
        Recompose();
    }

    Framebuffer* Canvas::GetActiveFramebuffer()
    {
        if (m_Layers.empty()) return nullptr;
        auto fbo = m_Layers[m_ActiveLayerIndex].GetActiveFramebuffer(m_CurrentFrame);
        return fbo.get();
    }

    void Canvas::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0) return;
        if (width == m_Width && height == m_Height) return;

        m_Width = width;
        m_Height = height;
        m_CompositeBuffer->Resize(width, height);

        for (auto& layer : m_Layers)
        {
            layer.Resize(width, height);
        }
        Recompose();
    }

    void Canvas::Recompose()
    {
        m_CompositeBuffer->Bind();

        // Clear to Empty (Transparent)
        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        Renderer2D::BeginScene(glm::ortho(0.0f, (float)m_Width, 0.0f, (float)m_Height));

        for (int i = 0; i < m_Layers.size(); i++)
        {
            auto& layer = m_Layers[i];

            // Skip invisible layers
            if (!layer.IsVisible) continue;
            if (layer.Opacity <= 0.01f) continue;

            // [CRITICAL] FLUSH before changing blend mode
            // If we don't flush, the previous layer might get drawn with the new blend mode!
            Renderer2D::Flush();

            // [NEW] APPLY BLEND MODE
            switch (layer.Blend)
            {
            case BlendMode::Normal:
                GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                GLCall(glBlendEquation(GL_FUNC_ADD));
                break;
            case BlendMode::Multiply:
                // Dst * Src
                GLCall(glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA));
                GLCall(glBlendEquation(GL_FUNC_ADD));
                break;
            case BlendMode::Add:
                // Dst + Src (Glow)
                GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
                GLCall(glBlendEquation(GL_FUNC_ADD));
                break;
            case BlendMode::Screen:
                // 1 - (1-Dst)*(1-Src)
                GLCall(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR));
                GLCall(glBlendEquation(GL_FUNC_ADD));
                break;
            case BlendMode::Overlay:
                // Fallback for now (Standard Normal)
                GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                break;
            }

            // --- ONION SKIN RENDERING ---
            if (i == m_ActiveLayerIndex && m_OnionSkinEnabled)
            {
                auto prevFBO = layer.GetPreviousFramebuffer(m_CurrentFrame);
                if (prevFBO)
                {
                    Renderer2D::DrawQuad(
                        { 0,0 }, { m_Width, m_Height },
                        prevFBO->GetTextureID(),
                        1.0f,
                        { 1.0f, 0.5f, 0.5f, 0.3f * layer.Opacity }
                    );
                }
                auto nextFBO = layer.GetNextFramebuffer(m_CurrentFrame);
                if (nextFBO)
                {
                    Renderer2D::DrawQuad(
                        { 0,0 }, { m_Width, m_Height },
                        nextFBO->GetTextureID(),
                        1.0f,
                        { 0.5f, 0.5f, 1.0f, 0.3f * layer.Opacity }
                    );
                }
            }

            // --- MAIN FRAME RENDERING ---
            auto fbo = layer.GetActiveFramebuffer(m_CurrentFrame);
            if (fbo)
            {
                // Apply Layer Opacity to the Tint Color
                Renderer2D::DrawQuad(
                    { 0,0 },
                    { m_Width, m_Height },
                    fbo->GetTextureID(),
                    1.0f,
                    { 1.0f, 1.0f, 1.0f, layer.Opacity }
                );
            }
        }

        Renderer2D::EndScene();

        // [CRITICAL] Reset Blend Mode to Normal for UI
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        m_CompositeBuffer->Unbind();
    }

    void Canvas::SetOnionSkinEnabled(bool enabled)
    {
        m_OnionSkinEnabled = enabled;
        Recompose();
    }
}