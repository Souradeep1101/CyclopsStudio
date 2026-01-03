#pragma once

#include "Canvas.h"
#include "ImGui.h"
#include "BrushEngine.h"
#include "Layer.h"

#include <memory>
#include <cstdint>

namespace Cyclops
{
	/**
	 * @class EditorLayer
	 * @brief The primary Application Layer.
	 * * @details This layer contains the specific logic for the Cyclops Studio Editor.
	 * It manages the Layout, the Viewport, the Canvas Document, and User Inputs.
	 * It inherits from Layer, so it is updated automatically by the Engine.
	 */
	class EditorLayer : public Layer
	{
	private:
		// --- Core Systems (Owned) ---
		std::unique_ptr<Canvas> m_Canvas;
		std::unique_ptr<BrushEngine> m_BrushEngine;

		// --- Editor State ---
		float m_ZoomLevel = 1.0f;
		ImVec2 m_PanOffset = { 0.0f, 0.0f };

		// --- Document (Canvas) Settings ---
		uint32_t m_CanvasWidth = 800;
		uint32_t m_CanvasHeight = 600;

		// --- Input State ---
		ImVec2 m_LastMousePos = { 0.0f, 0.0f };
		bool m_isDrawing = false;

		// Internal UI Helpers
		void DrawViewport();
		void DrawTools();

	public:
		EditorLayer();
		~EditorLayer();

		/*void Init();
		void OnImGuiRender();
		void OnUpdate(float ts);*/

		/**
		 * @brief Called when the layer is added to the Engine.
		 * @details Initializes the Canvas and BrushEngine.
		 */
		virtual void OnAttach() override; // <-- New Name

		/**
		 * @brief Called when the layer is removed.
		 * @details Resources (unique_ptrs) are freed automatically here.
		 */
		virtual void OnDetach() override;
		
		/**
		 * @brief The main UI render loop.
		 * @details Draws the Dockspace, Viewport, and Toolbar windows.
		 */
		virtual void OnImGuiRender() override;
		
		/**
		 * @brief The logic update loop.
		 */
		virtual void OnUpdate(float ts) override;
		
		/**
		 * @brief Handles global events (Keyboard shortcuts, etc.).
		 * @note Mouse clicks inside the viewport are handled in OnImGuiRender to respect UI layering.
		 */
		virtual void OnEvent(Event& event) override;
	};
}