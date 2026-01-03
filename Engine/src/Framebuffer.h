#pragma once

#include <cstdint>

namespace Cyclops
{
	/**
	 * @class Framebuffer
	 * @brief Wrapper around an OpenGL Framebuffer Object (FBO).
	 * * @details An FBO allows rendering to a texture instead of the screen.
	 * This is critical for the "Canvas" system, as it allows us to zoom, pan,
	 * and rotate the drawing surface inside an ImGui window.
	 */
	class Framebuffer
	{
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_TextureID = 0;
		uint32_t m_Width, m_Height;

		/**
		 * @brief Internal helper to delete old GL objects and generate new ones.
		 */
		void Invalidate();

	public:
		/**
		 * @brief Creates an FBO and allocates GPU memory for the texture attachment.
		 */
		Framebuffer(uint32_t width, uint32_t height);

		/**
		 * @brief Destructor. Automatically deletes the GL resources.
		 */
		~Framebuffer();

		// Safety: FBOs represent unique GPU resources. Copying is dangerous.
		Framebuffer(const Framebuffer&) = delete;
		Framebuffer& operator=(const Framebuffer&) = delete;

		/**
		 * @brief Binds this FBO as the current render target.
		 * All subsequent OpenGL draw calls will render to this framebuffer's texture.
		 */
		void Bind() const;
		
		/**
		 * @brief Unbinds the FBO, returning rendering to the default window (screen).
		 */
		void Unbind() const;
		
		/**
		 * @brief Re-allocates the texture attachments with new dimensions.
		 */
		void Resize(uint32_t width, uint32_t height);
		
		// --- Getters ---

		/** @return The internal OpenGL ID for the FBO. */
		uint32_t GetRendererID() const { return m_RendererID; }

		/** @return The internal OpenGL ID for the Color Attachment Texture. Use this for ImGui::Image(). */
		uint32_t GetTextureID() const { return m_TextureID; }
		
		/** @return The width in pixels. */
		uint32_t GetWidth() const { return m_Width; }
		
		/** @return The height in pixels. */
		uint32_t GetHeight() const { return m_Height; }
	};
}