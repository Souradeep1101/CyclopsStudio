#pragma once

#include <imgui.h>

namespace Cyclops::UI {

    // A central place for our visual style
    struct Theme
    {
        // Standard Colors (Dark Mode)
        static constexpr ImVec4 Background = { 0.15f, 0.15f, 0.15f, 1.0f };
        static constexpr ImVec4 BackgroundDark = { 0.1f,  0.1f,  0.1f,  1.0f };

        static constexpr ImVec4 Text = { 0.9f,  0.9f,  0.9f,  1.0f };
        static constexpr ImVec4 TextDisabled = { 0.5f,  0.5f,  0.5f,  1.0f };

        // Accents (The "Cyclops" Green)
        static constexpr ImVec4 Accent = { 0.2f,  0.8f,  0.2f,  1.0f };
        static constexpr ImVec4 AccentDim = { 0.2f,  0.6f,  0.2f,  1.0f };

        // Selection
        static constexpr ImVec4 Selection = { 0.26f, 0.59f, 0.98f, 0.4f }; // Soft Blue
    };

    // Helper to standardize spacing
    inline void ShiftCursor(float x, float y)
    {
        ImVec2 pos = ImGui::GetCursorPos();
        ImGui::SetCursorPos({ pos.x + x, pos.y + y });
    }
}