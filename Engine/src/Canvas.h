#pragma once

#include <cstdint>
#include"Framebuffer.h"
#include <memory>

namespace Cyclops
{
	/**
	 * @class Canvas
	 * @brief Represents the logical "Document" or "Image" being edited.
	 * * @details The Canvas acts as the high-level manager for the underlying Framebuffer.
	 * It handles resizing logic and exposes the texture ID for ImGui rendering.
	 */
	class Canvas
	{
	private:
		uint32_t m_Width, m_Height;

		// RAII: The Canvas is the sole owner of its Framebuffer.
		std::unique_ptr<Framebuffer> m_ActiveFramebuffer;

		// std::vector<Layer*> m_Layers;

	public:
		/**
		 * @brief Creates a new blank Canvas.
		 * @param width Initial width in pixels.
		 * @param height Initial height in pixels.
		 */
		Canvas(uint32_t width, uint32_t height);
		~Canvas();

		/**
		 * @brief Resizes the underlying framebuffer.
		 * @note This is a destructive operation (clears the image).
		 */
		void Resize(uint32_t width, uint32_t height);

		// --- Getters ---

		/** * @return The OpenGL Texture ID of the canvas content.
		 * @note Pass this ID to ImGui::Image() to display the canvas in the editor.
		 */
		uint32_t GetTextureID() const { return m_ActiveFramebuffer->GetTextureID(); }

		/**
		 * @brief Grants access to the internal Framebuffer for drawing.
		 * @return Raw pointer to the framebuffer.
		 * @warning Do NOT delete this pointer. The Canvas retains ownership.
		 */
		Framebuffer* GetActiveFramebuffer() const { return m_ActiveFramebuffer.get(); }

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
	};
}
