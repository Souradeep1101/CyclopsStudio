#include "Framebuffer.h"
#include "OpenGLDebug.h"

namespace Cyclops
{
	void Framebuffer::Invalidate()
	{
		// Delete previous objects if they exist
		if (m_RendererID)
		{
			GLCall(glDeleteFramebuffers(1, &m_RendererID));
			GLCall(glDeleteTextures(1, &m_TextureID));
			m_RendererID = 0;
			m_TextureID = 0;
		}

		// Create Framebuffer
		GLCall(glGenFramebuffers(1, &m_RendererID));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));

		// Create Texture (Canvas)
		GLCall(glGenTextures(1, &m_TextureID));
		GLCall(glBindTexture(GL_TEXTURE_2D, m_TextureID));

		// Allocate memory and not fill it
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

		// Essential Texture Parameters
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		// Attach texture to framebuffer
		GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0));

		// Validation Check
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

		// Unbind the framebuffer
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	Framebuffer::Framebuffer(uint32_t width, uint32_t height)
		: m_Width{ width }, m_Height{ height }, m_RendererID{ 0 }, m_TextureID{ 0 }
	{
		// Call helper function
		Invalidate();
	}

	Framebuffer::~Framebuffer()
	{
		if (m_RendererID)
			GLCall(glDeleteFramebuffers(1, &m_RendererID));
		if (m_TextureID)
			GLCall(glDeleteTextures(1, &m_TextureID));
	}

	void Framebuffer::Bind() const
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));
		GLCall(glViewport(0, 0, m_Width, m_Height));
	}

	void Framebuffer::Unbind() const
	{
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	void Framebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
			return;

		if (width == m_Width && height == m_Height)
			return;

		m_Width = width;
		m_Height = height;

		// Call helper function
		Invalidate();
	}
}
