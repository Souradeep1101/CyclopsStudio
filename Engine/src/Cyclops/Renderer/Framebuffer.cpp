#include "Cyclops/Renderer/Framebuffer.h"
#include "Cyclops/Platform/OpenGL/OpenGLFramebuffer.h"

namespace Cyclops
{
    std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
    {
        // This is the ONLY code that should be in this file.
        // It creates the specific implementation (OpenGL).
        return std::make_shared<OpenGLFramebuffer>(spec);
    }
}