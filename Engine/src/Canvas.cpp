#include "Canvas.h"
#include "OpenGLDebug.h"

#include <glad/glad.h>

namespace Cyclops
{
	Canvas::Canvas(uint32_t width, uint32_t height)
		: m_Width{ width }, m_Height{ height }
	{
		// Create GPU resource
		m_ActiveFramebuffer =  std::make_unique<Framebuffer>(m_Width, m_Height);

		// Initialize the canvas with white color (Paper color)
		m_ActiveFramebuffer->Bind();
		GLCall(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));
		m_ActiveFramebuffer->Unbind();
	}
	
	Canvas::~Canvas()
	{
	}
	
	void Canvas::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
			return;

		if (width == m_Width && height == m_Height)
			return;

		m_Width = width;
		m_Height = height;

		// Resize the underlying framebuffer
		// Note: This wipes the data in our current implementation!
		m_ActiveFramebuffer->Resize(m_Width, m_Height);

		// Re-clear to white after resize
		m_ActiveFramebuffer->Bind();
		GLCall(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));
		m_ActiveFramebuffer->Unbind();
	}
}
