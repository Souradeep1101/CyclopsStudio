#pragma once

#include "Framebuffer.h"
#include "Shader.h"
#include <cstdint>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Cyclops
{
	/**
	 * @struct BrushSettings
	 * @brief Container for dynamic brush parameters.
	 */
	struct BrushSettings
	{
		float Size = 20.0f;								/// Diameter of the brush in pixels.
		glm::vec4 Color = { 0.0f, 0.0f, 1.0f, 1.0f };	///< RGBA color.
		float Spacing = 0.1f;							///< Distance between interpolation dots (percentage of size).
		// Future: float Hardness, float Opacity, etc.
	};

	/**
	 * @class BrushEngine
	 * @brief The dedicated rendering system for drawing strokes.
	 * * @details Unlike the generic Renderer2D, this engine is optimized for
	 * high-density "Splatting". It interpolates mouse movements to draw
	 * continuous, smooth lines even when the mouse moves faster than the framerate.
	 * * @note Ownership: This class OWNS its internal Shader (via unique_ptr),
	 * but purely BORROWS the target Framebuffer (raw pointer).
	 */
	class BrushEngine
	{
	public:
		BrushEngine();
		~BrushEngine();

		/**
		 * @brief Loads the brush shader and generates the quad geometry (VAO/VBO).
		 */
		void Init();

		/**
		 * @brief Starts a new stroke sequence.
		 * @param target The Framebuffer to draw onto. The BrushEngine does NOT take ownership of this.
		 * @param x Initial X coordinate (Canvas Space).
		 * @param y Initial Y coordinate (Canvas Space).
		 */
		void BeginStroke(Framebuffer* target, float x, float y);

		/**
		 * @brief Continues the current stroke to a new position.
		 * @details Automatically calculates linear interpolation (LERP) between the last
		 * known position and this new position to fill gaps with dots.
		 * @param x New X coordinate.
		 * @param y New Y coordinate.
		 * @param pressure Stylus pressure (0.0 to 1.0). Currently unused, ready for Phase 4.
		 */
		void ContinueStroke(float x, float y, float pressure = 1.0f);

		/**
		 * @brief Finalizes the stroke and clears the current render target pointer.
		 */
		void EndStroke();

		// --- Getters ---
		
		/** @return Reference to the settings struct for real-time editing. */
		BrushSettings& GetSettings() { return m_Settings; }

		// --- Testers ---
		
		/**
		 * @brief Pure math helper for calculating intermediate points.
		 * @note Static and isolated for easy Unit Testing.
		 */
		static std::vector<glm::vec2> CalculateInterpolation(
			glm::vec2 start,
			glm::vec2 end,
			float spacing,
			float brushSize
		);

	private:
		/**
		 * @brief Internal draw call. Renders a single "Splat" (Quad) at the location.
		 */
		void PaintQuad(float x, float y);

		// RAII: Owns the shader exclusively.
		std::unique_ptr<Shader> m_Shader = nullptr;
		
		uint32_t m_VAO = 0, m_VBO = 0;

		BrushSettings m_Settings;
		
		// Non-owning reference (Observer)
		Framebuffer* m_CurrentTarget = nullptr;

		// State for interpolation
		glm::vec2 m_LastPos = { 0.0f, 0.0f };
	};
}