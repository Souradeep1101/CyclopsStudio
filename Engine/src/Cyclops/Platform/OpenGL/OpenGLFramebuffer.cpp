#include "Cyclops/Platform/OpenGL/OpenGLFramebuffer.h"
#include "Cyclops/Platform/OpenGL/OpenGLDebug.h" // Needed for GLCall

#include <glad/glad.h>
#include <iostream>

namespace Cyclops
{
    static const uint32_t s_MaxFramebufferSize = 8192;

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec)
    {
        Invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        GLCall(glDeleteFramebuffers(1, &m_RendererID));
        GLCall(glDeleteTextures(1, &m_ColorAttachment));
        GLCall(glDeleteTextures(1, &m_DepthAttachment));
    }

    void OpenGLFramebuffer::Invalidate()
    {
        if (m_RendererID)
        {
            GLCall(glDeleteFramebuffers(1, &m_RendererID));
            GLCall(glDeleteTextures(1, &m_ColorAttachment));
            GLCall(glDeleteTextures(1, &m_DepthAttachment));
            m_RendererID = 0;
            m_ColorAttachment = 0;
            m_DepthAttachment = 0;
        }

        GLCall(glCreateFramebuffers(1, &m_RendererID));
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));

        // Create Color Attachment
        GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachment));
        GLCall(glBindTexture(GL_TEXTURE_2D, m_ColorAttachment));
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Specification.Width, m_Specification.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorAttachment, 0));

        // Create Depth Attachment (Required for correct rendering sometimes)
        GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment));
        GLCall(glBindTexture(GL_TEXTURE_2D, m_DepthAttachment));
        GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_Specification.Width, m_Specification.Height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL));
        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_DepthAttachment, 0));

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer is incomplete!" << std::endl;

        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void OpenGLFramebuffer::Bind()
    {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID));
        GLCall(glViewport(0, 0, m_Specification.Width, m_Specification.Height));
    }

    void OpenGLFramebuffer::Unbind()
    {
        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
        {
            return;
        }

        m_Specification.Width = width;
        m_Specification.Height = height;

        Invalidate();
    }

    // --- NEW: Undo/Redo Implementation ---

    std::shared_ptr<Framebuffer> OpenGLFramebuffer::Clone()
    {
        // 1. Create a new Framebuffer with the exact same specs
        auto newFBO = std::make_shared<OpenGLFramebuffer>(m_Specification);

        // 2. Copy the data (Blit)
        newFBO->BlitFrom(shared_from_this());

        return newFBO;
    }

    void OpenGLFramebuffer::BlitFrom(const std::shared_ptr<Framebuffer>& source)
    {
        uint32_t sourceID = source->GetRendererID();
        uint32_t destID = this->GetRendererID();

        GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, sourceID));
        GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, destID));

        // Copy everything (Color Buffer 0)
        GLCall(glBlitFramebuffer(
            0, 0, m_Specification.Width, m_Specification.Height,
            0, 0, m_Specification.Width, m_Specification.Height,
            GL_COLOR_BUFFER_BIT,
            GL_NEAREST
        ));

        GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
}