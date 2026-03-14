#pragma once

#include "Cyclops/Scene/Canvas.h"

namespace Cyclops {

    class TimelinePanel
    {
    public:
        TimelinePanel() = default;

        void OnImGuiRender(Canvas* canvas, bool& isPlaying, int& fps, int& maxFrames);

    private:
        // UI Settings
        float m_FrameWidth = 20.0f; // Width of one frame in pixels (Zoom level)
        float m_HeaderHeight = 30.0f;
        float m_RowHeight = 25.0f;
    };
}