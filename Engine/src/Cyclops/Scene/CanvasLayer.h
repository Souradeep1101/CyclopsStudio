#pragma once

#include "Cyclops/Renderer/Framebuffer.h"
#include "Cyclops/Core/Keyframe.h" 
#include "Cyclops/Core/Base.h"
#include "Cyclops/Renderer/Renderer2D.h"

#include <string>
#include <memory>
#include <vector>
#include <map> 

namespace Cyclops
{
    // [CSP STANDARD] Industry Blend Modes
    enum class BlendMode
    {
        Normal = 0,
        Multiply,
        Add,      // "Linear Dodge" / Glow
        Screen,
        Overlay
    };

    class CYCLOPS_API CanvasLayer
    {
    public:
        // --- DATA & STATE ---
        std::string Name;

        // [CSP VISUALS]
        bool IsVisible = true;
        float Opacity = 1.0f;
        BlendMode Blend = BlendMode::Normal;

        // [CSP FLAGS]
        bool IsLocked = false;          // "Padlock": Prevents all editing/moving
        bool IsAlphaLocked = false;     // "Checkerboard": Only paint on existing pixels
        bool IsClippingMask = false;    // "Clip": Masks to the layer below (Logic to be added)

        // --- TIMELINE STORAGE ---
        // Map: Frame Number -> Keyframe Data
        std::map<int, std::shared_ptr<Keyframe>> Keyframes;

        // Visual Cache
        std::shared_ptr<Texture2D> Thumbnail;

        // --- CONSTRUCTORS ---
        CanvasLayer() = default;
        CanvasLayer(uint32_t width, uint32_t height, const std::string& name = "New Layer");

        // --- KEYFRAME LOGIC ---
        void CreateKeyframe(int frameIndex, uint32_t width, uint32_t height);
        void DuplicateKeyframe(int frameIndex);
        void RemoveKeyframe(int frameIndex);

        // Helper to update the thumbnail from a framebuffer
        void UpdateThumbnail(std::shared_ptr<Framebuffer> sourceFBO);

        // Getters
        std::shared_ptr<Framebuffer> GetActiveFramebuffer(int currentFrameIndex);
        std::shared_ptr<Framebuffer> GetPreviousFramebuffer(int currentFrameIndex);
        std::shared_ptr<Framebuffer> GetNextFramebuffer(int currentFrameIndex);

        // System
        void Resize(uint32_t width, uint32_t height);
    };
}