#pragma once

#include "Cyclops/Core/Base.h"

#include <cstdint>
#include <vector>
#include <memory>

namespace Cyclops
{
	// 1. Define available formats
	enum class FramebufferTextureFormat
	{
		None = 0,
		RGBA8,          // Standard Color
		Depth24Stencil8 // Depth/Stencil
	};

	// 2. The Specification Struct
	struct CYCLOPS_API FramebufferSpecification
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		std::vector<FramebufferTextureFormat> Formats;
		bool SwapChainTarget = false;
	};

	/**
	 * @class Framebuffer
	 * @brief Abstract Interface for a Framebuffer.
	 * @details This class defines the contract. Platform-specific implementations
	 * (like OpenGLFramebuffer) inherit from this.
	 */
	class CYCLOPS_API Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		// --- Core API ---
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		// --- Getters ---
		virtual uint32_t GetRendererID() const = 0;

		// Returns the texture ID for the specific attachment index (default 0)
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

		// Helper to keep your existing code working (Canvas calls GetTextureID)
		virtual uint32_t GetTextureID() const { return GetColorAttachmentRendererID(0); }

		virtual const FramebufferSpecification& GetSpecification() const = 0;

		// --- Undo/Redo System Requirements ---
		// Creates a perfect copy of this framebuffer (new texture ID, same data)
		virtual std::shared_ptr<Framebuffer> Clone() = 0;

		// Copies data FROM another framebuffer INTO this one
		virtual void BlitFrom(const std::shared_ptr<Framebuffer>& source) = 0;

		// --- Factory Method ---
		// This creates the actual OpenGLFramebuffer instance
		static std::shared_ptr<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}