#pragma once

#include "Cyclops/Renderer/Framebuffer.h"
#include "Cyclops/Core/Base.h"

#include <cstdint>

namespace Cyclops
{
	class CYCLOPS_API OpenGLFramebuffer : public Framebuffer, public std::enable_shared_from_this<OpenGLFramebuffer>
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		// Base Interface Implementation
		void Invalidate();
		virtual void Bind() override;
		virtual void Unbind() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { return m_ColorAttachment; }
		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

		// [NEW] Undo/Redo System Requirements
		virtual std::shared_ptr<Framebuffer> Clone() override;
		virtual void BlitFrom(const std::shared_ptr<Framebuffer>& source) override;

	private:
		uint32_t m_RendererID = 0;
		uint32_t m_ColorAttachment = 0;
		uint32_t m_DepthAttachment = 0;
		FramebufferSpecification m_Specification;
	};
}