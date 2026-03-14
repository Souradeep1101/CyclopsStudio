#pragma once

#include "Cyclops/Scene/Canvas.h"
#include "Cyclops/Tools/BrushEngine.h"
#include "Cyclops/Core/Layer.h"

// Panels
#include "Panels/LayerPanel.h"
#include "Panels/TimelinePanel.h"
#include "Panels/ViewportPanel.h"

#include <imgui.h>
#include <memory>
#include <vector>
#include <string>

namespace Cyclops
{
    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        ~EditorLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnImGuiRender() override;
        virtual void OnUpdate(float ts) override;
        virtual void OnEvent(Event& event) override;

    private:
        // Internal Helpers
        void DrawPanels();
        void ImportGengaSequence(const std::string& path);

        // [FIX] Add this missing declaration!
        void ResetProject();

    private:
        // Core Systems
        std::unique_ptr<Canvas> m_Canvas;
        std::unique_ptr<BrushEngine> m_BrushEngine;
        std::vector<std::shared_ptr<Tool>> m_Tools;
        Tool* m_ActiveTool = nullptr;

        // Document Settings
        uint32_t m_CanvasWidth = 800;
        uint32_t m_CanvasHeight = 600;

        // Playback State
        bool m_IsPlaying = false;
        float m_TimeAccumulator = 0.0f;
        int m_FrameRate = 12;
        int m_MaxFrames = 24;

        // UI Panels
        LayerPanel m_LayerPanel;
        TimelinePanel m_TimelinePanel;
        ViewportPanel m_ViewportPanel;
    };
}