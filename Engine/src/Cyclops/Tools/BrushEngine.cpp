#include "Cyclops/Tools/BrushEngine.h"
#include "Cyclops/Renderer/Renderer2D.h"
#include "Cyclops/Platform/OpenGL/OpenGLDebug.h"

#include <iostream>
#include <glad/glad.h>
#include <algorithm> // for std::max

namespace Cyclops
{
	BrushEngine::BrushEngine()
	{
		m_CurrentBrush.Name = "Default Pen";
		m_CurrentBrush.Size = 30.0f;
		m_CurrentBrush.Color = { 0.0f, 0.0f, 0.0f, 1.0f };
		m_CurrentBrush.Spacing = 0.15f;
	}

	void BrushEngine::EndStroke()
	{
		m_IsDrawing = false;
		m_PathResidue = 0.0f; // Reset residue so next stroke starts fresh
	}

	void BrushEngine::DrawDab(const glm::vec2& pos, float size)
	{
		glm::vec2 centeredPos = pos - glm::vec2(size * 0.5f);

		if (m_CurrentBrush.Type == BrushType::Textured && m_CurrentBrush.Texture)
		{
			Renderer2D::DrawQuad(centeredPos, { size, size }, m_CurrentBrush.Texture, 1.0f, m_CurrentBrush.Color);
		}
		else
		{
			Renderer2D::DrawQuad(centeredPos, { size, size }, m_CurrentBrush.Color);
		}
	}

	void BrushEngine::Paint(const glm::vec2& currentInputPos, float pressure)
	{
		// 1. Start of Stroke (No previous point to curve from)
		if (!m_IsDrawing)
		{
			m_IsDrawing = true;
			m_LastPos = currentInputPos;       // Ink starts here
			m_LastInputPos = currentInputPos;  // Mouse starts here
			m_PathResidue = 0.0f;

			float size = m_CurrentBrush.Size;
			if (m_CurrentBrush.UsePressureSize) size *= pressure;

			// Draw the very first dot immediately
			DrawDab(currentInputPos, size);
			return;
		}

		// 2. SMOOTHING LOGIC (Quadratic Bezier)
		// Instead of drawing a straight line to the mouse, we curve towards the midpoint.
		// Control Point = The mouse position from the LAST frame.
		// End Point     = The midpoint between LAST frame and CURRENT frame.
		glm::vec2 controlPoint = m_LastInputPos;
		glm::vec2 endPoint = (m_LastInputPos + currentInputPos) * 0.5f;

		// Draw the curved segment
		PaintQuadraticBezier(m_LastPos, controlPoint, endPoint, pressure);

		// 3. Update State
		// The "Ink" is now waiting at the midpoint
		m_LastPos = endPoint;
		// The "Mouse" history updates to the current real input
		m_LastInputPos = currentInputPos;
	}

	void BrushEngine::PaintQuadraticBezier(const glm::vec2& start, const glm::vec2& control, const glm::vec2& end, float pressure)
	{
		// ... (Length and Spacing calculation stay the same) ...
		glm::vec2 d1 = control - start;
		glm::vec2 d2 = end - control;
		float len = glm::length(d1) + glm::length(d2);

		if (len <= 0.1f) return;

		float size = m_CurrentBrush.Size;
		if (m_CurrentBrush.UsePressureSize) size *= pressure;
		float spacing = std::max(m_CurrentBrush.Spacing * size, 1.0f);

		int segments = (int)(len / 2.0f);
		if (segments < 1) segments = 1;

		// [FIX 1] Remove 'distToNext' declaration from here
		// float distToNext = spacing - m_PathResidue; <--- DELETE THIS LINE

		glm::vec2 prevP = start;

		for (int i = 1; i <= segments; i++)
		{
			float t = (float)i / (float)segments;
			float u = 1.0f - t;
			float tt = t * t;
			float uu = u * u;

			glm::vec2 p = (uu * start) + (2 * u * t * control) + (tt * end);
			glm::vec2 direction = p - prevP;
			float dist = glm::length(direction);

			if (dist > 0.0001f)
			{
				glm::vec2 stepDir = direction / dist;
				float currentDist = 0.0f;

				// [FIX 2] Recalculate 'distToNext' for EVERY segment based on the current residue
				float distToNext = spacing - m_PathResidue;

				while (currentDist + distToNext <= dist)
				{
					currentDist += distToNext;
					glm::vec2 drawPos = prevP + (stepDir * currentDist);

					DrawDab(drawPos, size);

					m_PathResidue = 0.0f;
					distToNext = spacing; // Reset for subsequent dots in THIS same segment
				}

				m_PathResidue += (dist - currentDist);
			}

			prevP = p;
		}
	}
}