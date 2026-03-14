#pragma once

#include "Cyclops/Scene/Canvas.h"
#include "Cyclops/Tools/BrushEngine.h"
#include "Cyclops/Tools/Tool.h"
#include "Cyclops/Renderer/Framebuffer.h"

#include <glm/glm.hpp>
#include <imgui.h>
#include <memory>

namespace Cyclops {

    class ViewportPanel
    {
    public:
        ViewportPanel() = default;

        // The Main Render Function
        // We pass in the Core Systems so the Panel can interact with them
        void OnImGuiRender(Canvas* canvas, BrushEngine* brushEngine, Tool* activeTool);

        // Getters
        float GetZoomLevel() const { return m_ZoomLevel; }
        void ResetView() { m_ZoomLevel = 1.0f; m_PanOffset = { 0.0f, 0.0f }; }

    private:
        // Helper to convert screen coordinates to canvas pixels
        glm::vec2 GetMousePosInCanvas(Canvas* canvas);

        // --- Viewport State ---
        float m_ZoomLevel = 1.0f;
        ImVec2 m_PanOffset = { 0.0f, 0.0f };
        glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
        
        // We need to store where the canvas is on screen to calculate mouse relative position
        glm::vec2 m_CanvasScreenPos = { 0.0f, 0.0f }; 

        // --- Painting State ---
        // Moved from EditorLayer because the Viewport handles input
        bool m_IsDrawingStroke = false;
        std::shared_ptr<Framebuffer> m_BeforeStrokeSnapshot;
    };
}