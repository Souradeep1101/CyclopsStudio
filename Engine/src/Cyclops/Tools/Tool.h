#pragma once

#include "Cyclops/Platform/OpenGL/OpenGLDebug.h"
#include "Cyclops/Core/Base.h"

#include <glm/glm.hpp>
#include <string>

namespace Cyclops
{
    // Keep this Enum for UI IDs.
    enum class ToolType
    {
        None = 0,
        Brush,
        Eraser
    };

    class CYCLOPS_API Tool
    {
    public:
        virtual ~Tool() = default;

        // Metadata
        virtual std::string GetName() const = 0;
        virtual ToolType GetType() const = 0;

        // Lifecycle Events
        virtual void OnActivate() {}
        virtual void OnDeactivate() {}

        // Operation Events
        // We return 'true' if the tool did something (dirty flag)
        virtual bool OnPaint(const glm::vec2& pos, float pressure) = 0;
        virtual void OnEndStroke() {}

        // Configuration
        // Should this tool change how the GPU blends pixels?
        virtual void ConfigureBlendState() const
        {
            // Default: Normal Blending
            GLCall(glEnable(GL_BLEND));
            GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            GLCall(glBlendEquation(GL_FUNC_ADD));
        }
    };
}