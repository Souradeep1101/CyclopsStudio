#pragma once

#include "Cyclops/Tools/Brush.h"
#include "Cyclops/Tools/Tool.h"
#include "Cyclops/Core/Base.h"

#include <glm/glm.hpp>
#include <vector>

namespace Cyclops
{
	class CYCLOPS_API BrushEngine
	{
	public:
		BrushEngine();

		void Init() {}

		// Main function called by EditorLayer
		void Paint(const glm::vec2& position, float pressure = 1.0f);
		void EndStroke();

		// Settings
		void SetBrush(const Brush& brush) { m_CurrentBrush = brush; }
		Brush& GetBrush() { return m_CurrentBrush; }

		void SetTool(ToolType tool) { m_ActiveTool = tool; }
		ToolType GetTool() const { return m_ActiveTool; }

	private:
		// Helper to actually render a single dot
		void DrawDab(const glm::vec2& pos, float size);

		// Helper to interpolate smoothly along a curve
		void PaintQuadraticBezier(const glm::vec2& start, const glm::vec2& control, const glm::vec2& end, float pressure);

	private:
		Brush m_CurrentBrush;
		ToolType m_ActiveTool = ToolType::Brush;

		// --- Smoothing State ---
		glm::vec2 m_LastPos = { 0.0f, 0.0f };      // Where the "Ink" stopped drawing
		glm::vec2 m_LastInputPos = { 0.0f, 0.0f }; // Where the "Mouse" actually was last frame

		bool m_IsDrawing = false;
		float m_PathResidue = 0.0f;                // "Leftover" distance from previous frame
	};
}