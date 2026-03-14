#include "Panels/ViewportPanel.h"

#include "Cyclops/Core/Engine.h"
#include "Cyclops/Platform/OpenGL/OpenGLDebug.h"
#include "Cyclops/Core/Profiling/Instrumentor.h"
#include "Cyclops/Renderer/Renderer2D.h"
#include "Cyclops/Core/CommandHistory.h"
#include "Cyclops/Tools/PaintCommand.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

namespace Cyclops {

    void ViewportPanel::OnImGuiRender(Canvas* canvas, BrushEngine* brushEngine, Tool* activeTool)
    {
        CYCLOPS_PROFILE_FUNCTION();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");

        // --- 1. Viewport Setup ---
        ImVec2 viewportMin = ImGui::GetCursorScreenPos();
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportSize.x, viewportSize.y };

        // Store for Mouse Calculation
        m_CanvasScreenPos = { viewportMin.x, viewportMin.y };

        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsWindowHovered())
        {
            if (io.MouseWheel != 0.0f)
            {
                m_ZoomLevel += io.MouseWheel * 0.1f;
                if (m_ZoomLevel < 0.1f) m_ZoomLevel = 0.1f;
            }
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle))
            {
                m_PanOffset.x += io.MouseDelta.x;
                m_PanOffset.y += io.MouseDelta.y;
            }
        }

        // --- 2. Calculate Geometry ---
        float canvasW = (float)canvas->GetWidth();
        float canvasH = (float)canvas->GetHeight();
        ImVec2 imageSize = { canvasW * m_ZoomLevel, canvasH * m_ZoomLevel };

        float xOffset = (viewportSize.x - imageSize.x) * 0.5f;
        float yOffset = (viewportSize.y - imageSize.y) * 0.5f;

        ImVec2 pMin = {
            viewportMin.x + xOffset + m_PanOffset.x,
            viewportMin.y + yOffset + m_PanOffset.y
        };
        ImVec2 pMax = { pMin.x + imageSize.x, pMin.y + imageSize.y };

        // Update Screen Pos specifically to the image top-left
        m_CanvasScreenPos = { pMin.x, pMin.y };

        // --- 3. Render Canvas ---
        // Background
        ImGui::GetWindowDrawList()->AddRectFilled(pMin, pMax, IM_COL32(50, 50, 50, 255));

        // Texture
        ImGui::GetWindowDrawList()->AddImage(
            (void*)(intptr_t)canvas->GetTextureID(),
            pMin, pMax,
            ImVec2(0, 1), ImVec2(1, 0)
        );

        // --- 4. Painting Logic ---
        glm::vec2 mousePixel = GetMousePosInCanvas(canvas);
        bool isInsideCanvas = (mousePixel.x >= 0 && mousePixel.x < canvasW &&
            mousePixel.y >= 0 && mousePixel.y < canvasH);

        // Start Stroke
        if (isInsideCanvas && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered())
        {
            int activeIdx = canvas->GetActiveLayerIndex();
            auto& layer = canvas->GetLayers()[activeIdx];

            // Lock Check
            if (!layer.IsLocked)
            {
                Framebuffer* activeLayer = canvas->GetActiveFramebuffer();
                if (activeLayer)
                {
                    m_BeforeStrokeSnapshot = activeLayer->Clone();
                    m_IsDrawingStroke = true;
                }
            }
        }

        // Continue Stroke
        if (m_IsDrawingStroke && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            if (isInsideCanvas)
            {
                int activeIdx = canvas->GetActiveLayerIndex();
                auto& layer = canvas->GetLayers()[activeIdx];

                // Guard Clause: Lock
                if (layer.IsLocked)
                {
                    m_IsDrawingStroke = false;
                    m_BeforeStrokeSnapshot = nullptr;
                }
                else
                {
                    Framebuffer* activeLayerBuffer = canvas->GetActiveFramebuffer();
                    if (activeLayerBuffer)
                    {
                        activeLayerBuffer->Bind();

                        // Alpha Lock Logic
                        if (layer.IsAlphaLocked)
                            glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                        else
                            activeTool->ConfigureBlendState();

                        glm::mat4 camera = glm::ortho(0.0f, canvasW, 0.0f, canvasH, -1.0f, 1.0f);
                        Renderer2D::BeginScene(camera);

                        activeTool->OnPaint({ mousePixel.x, mousePixel.y }, 1.0f);

                        Renderer2D::EndScene();

                        // Reset Blend
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                        activeLayerBuffer->Unbind();
                        canvas->Recompose();
                    }
                }
            }
        }
        else
        {
            brushEngine->EndStroke();
        }

        // End Stroke
        if (m_IsDrawingStroke && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            if (m_BeforeStrokeSnapshot)
            {
                int layerIndex = canvas->GetActiveLayerIndex();
                int frameIndex = canvas->GetCurrentFrame();

                auto cmd = std::make_shared<PaintCommand>(
                    canvas,
                    m_BeforeStrokeSnapshot,
                    layerIndex,
                    frameIndex
                );

                CommandHistory::AddCommand(cmd, false);
                m_BeforeStrokeSnapshot = nullptr;
            }
            m_IsDrawingStroke = false;
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    glm::vec2 ViewportPanel::GetMousePosInCanvas(Canvas* canvas)
    {
        auto [mx, my] = ImGui::GetMousePos();
        float mouseRelX = mx - m_CanvasScreenPos.x;
        float mouseRelY = my - m_CanvasScreenPos.y;
        float pixelX = mouseRelX / m_ZoomLevel;
        float pixelY = mouseRelY / m_ZoomLevel;

        // Y-Flip because OpenGL is bottom-left, Screen is top-left
        pixelY = (float)canvas->GetHeight() - pixelY;

        return { pixelX, pixelY };
    }
}