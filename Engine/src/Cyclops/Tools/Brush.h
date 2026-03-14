#pragma once

#include "Cyclops/Renderer/Texture.h" // Needed for the texture pointer
#include "Cyclops/Core/Base.h"

#include <string>
#include <glm/glm.hpp>
#include <memory>

namespace Cyclops
{
    enum class BrushType
    {
        BasicCircle,
        Textured
    };

    struct CYCLOPS_API Brush
    {
        std::string Name = "New Brush";
        BrushType Type = BrushType::BasicCircle;

        // --- Appearance ---
        std::shared_ptr<Texture2D> Texture; // The "Tip" image
        glm::vec4 Color = { 0.0f, 0.0f, 0.0f, 1.0f }; // Default Black
        float Size = 50.0f;
        float Hardness = 1.0f; // 0=Soft edge, 1=Hard edge (Future feature)

        // --- Behavior ---
        float Spacing = 0.1f; // 10% of size (Critical for smooth lines)
        float Opacity = 1.0f;

        // --- Dynamics ---
        bool UsePressureSize = true;
        bool UsePressureOpacity = true;

        // Helper to load a texture easily
        void SetTexture(const std::string& filepath)
        {
            Texture = std::make_shared<Texture2D>(filepath);
            Type = BrushType::Textured;
        }
    };
}