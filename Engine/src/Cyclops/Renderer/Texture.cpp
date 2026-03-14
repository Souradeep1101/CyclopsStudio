#include "Cyclops/Renderer/Texture.h"
#include "Cyclops/Platform/OpenGL/OpenGLDebug.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Cyclops
{
	// --- Constructor A: Load from File ---
	Texture2D::Texture2D(const std::string& path)
		: m_Path(path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);

		// [FIX 1] Force 4 channels (RGBA) by passing '4' as the last argument.
		// This converts Grayscale (1-channel) or RGB (3-channel) images to RGBA automatically.
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

		if (data)
		{
			m_Width = (uint32_t)width;
			m_Height = (uint32_t)height;

			// [FIX 2] Hardcode formats to RGBA since we forced it above.
			// This prevents the "GL_INVALID_ENUM" error for greyscale brushes.
			m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;

			GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID));
			GLCall(glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height));

			// [OPTIONAL POLISH] Use Linear filtering for smoother brushes
			GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			// [OPTIONAL POLISH] Clamp to edge so the brush doesn't wrap around
			GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

			GLCall(glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data));

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << path << std::endl;
		}
	}

	// --- Constructor B: Manual Creation ---
	Texture2D::Texture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height)
	{
		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		GLCall(glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID));
		GLCall(glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height));

		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT));
		GLCall(glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT));
	}

	Texture2D::~Texture2D()
	{
		GLCall(glDeleteTextures(1, &m_RendererID));
	}

	void Texture2D::SetData(void* data, uint32_t size)
	{
		// Note: Usually we check if size matches m_Width * m_Height * bpp
		GLCall(glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data));
	}

	void Texture2D::Bind(uint32_t slot) const
	{
		GLCall(glBindTextureUnit(slot, m_RendererID));
	}
}