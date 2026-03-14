#pragma once

#include "Cyclops/Renderer/Framebuffer.h"
#include "Cyclops/Core/Base.h"

#include <memory>

namespace Cyclops
{
    struct CYCLOPS_API Keyframe
    {
        // Where does this drawing start on the timeline?
        int FrameIndex = 0;

        // The actual pixel data for this frame
        std::shared_ptr<Framebuffer> Data;

        // Constructor
        Keyframe(int frame, std::shared_ptr<Framebuffer> data)
            : FrameIndex(frame), Data(data) {
        }
    };
}