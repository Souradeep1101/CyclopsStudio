#include "Cyclops/Scene/CanvasLayer.h"

#include "Cyclops/Platform/OpenGL/OpenGLDebug.h" 
#include <glad/glad.h> 
#include <iostream> 

namespace Cyclops
{
    CanvasLayer::CanvasLayer(uint32_t width, uint32_t height, const std::string& name)
        : Name(name)
    {
        // Automatically create the start frame at 0
        CreateKeyframe(0, width, height);
    }

    void CanvasLayer::CreateKeyframe(int frameIndex, uint32_t width, uint32_t height)
    {
        if (Keyframes.find(frameIndex) != Keyframes.end())
            return;

        FramebufferSpecification spec;
        spec.Width = width;
        spec.Height = height;
        spec.Formats = { FramebufferTextureFormat::RGBA8 };

        auto fbo = Framebuffer::Create(spec);

        // Clear to transparent
        fbo->Bind();
        GLCall(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        fbo->Unbind();

        Keyframes[frameIndex] = std::make_shared<Keyframe>(frameIndex, fbo);
    }

    void CanvasLayer::DuplicateKeyframe(int frameIndex)
    {
        // Safety: Don't overwrite existing
        if (Keyframes.find(frameIndex) != Keyframes.end()) return;

        // Find Source (Hold previous)
        auto sourceFBO = GetActiveFramebuffer(frameIndex);

        if (!sourceFBO) return; // Should handle empty case properly in real app

        // Deep Copy
        auto newFBO = sourceFBO->Clone();

        Keyframes[frameIndex] = std::make_shared<Keyframe>(frameIndex, newFBO);
    }

    void CanvasLayer::RemoveKeyframe(int frameIndex)
    {
        Keyframes.erase(frameIndex);
    }

    std::shared_ptr<Framebuffer> CanvasLayer::GetActiveFramebuffer(int currentFrameIndex)
    {
        // Find first element >= currentFrameIndex
        auto it = Keyframes.lower_bound(currentFrameIndex);

        if (it != Keyframes.end() && it->first == currentFrameIndex)
            return it->second->Data;

        // If at start, nothing before us
        if (it == Keyframes.begin())
            return nullptr;

        // Go back one to find the "holding" frame
        --it;
        return it->second->Data;
    }

    std::shared_ptr<Framebuffer> CanvasLayer::GetPreviousFramebuffer(int currentFrameIndex)
    {
        auto it = Keyframes.lower_bound(currentFrameIndex);
        if (it == Keyframes.begin()) return nullptr;
        --it;
        return it->second->Data;
    }

    std::shared_ptr<Framebuffer> CanvasLayer::GetNextFramebuffer(int currentFrameIndex)
    {
        auto it = Keyframes.upper_bound(currentFrameIndex);
        if (it == Keyframes.end()) return nullptr;
        return it->second->Data;
    }

    void CanvasLayer::Resize(uint32_t width, uint32_t height)
    {
        for (auto& [frameIdx, keyframe] : Keyframes)
        {
            if (keyframe->Data)
                keyframe->Data->Resize(width, height);
        }
    }
}