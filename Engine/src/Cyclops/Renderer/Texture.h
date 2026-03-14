#pragma once

#include "Cyclops/Core/Base.h"

#include <string>
#include <glad/glad.h>
#include <cstdint>

namespace Cyclops
{
	class CYCLOPS_API Texture2D
	{
	public:
		// Constructor for loading from disk
		Texture2D(const std::string& path);

		// Constructor for manual data (e.g., White Texture)
		Texture2D(uint32_t width, uint32_t height);

		~Texture2D();

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		uint32_t GetRendererID() const { return m_RendererID; }

		// Upload raw data to the GPU (Used for the 1x1 white texture)
		void SetData(void* data, uint32_t size);

		void Bind(uint32_t slot = 0) const;

		bool operator==(const Texture2D& other) const
		{
			return m_RendererID == other.m_RendererID;
		}

	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};
}