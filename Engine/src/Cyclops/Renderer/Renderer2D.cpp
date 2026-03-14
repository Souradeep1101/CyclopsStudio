#include "Cyclops/Renderer/Renderer2D.h"
#include "Cyclops/Renderer/Shader.h"
#include "Cyclops/Renderer/Texture.h"
#include "Cyclops/Platform/OpenGL/OpenGLDebug.h"

#include <memory>
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <filesystem>

namespace Cyclops
{
	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
	};

	struct Renderer2DData
	{
		// --- Limits ---
		static const uint32_t MaxQuads = 50000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32;

		// --- GPU Objects ---
		uint32_t QuadVAO = 0;
		uint32_t QuadVBO = 0;
		// [Safety] Track the IBO so we can delete it properly
		uint32_t QuadIBO = 0;

		// --- CPU Scratchpad ---
		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		// --- Resources ---
		std::unique_ptr<Shader> TextureShader;
		std::shared_ptr<Texture2D> WhiteTexture;

		// SUBMISSION
		std::array<uint32_t, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1;

		Renderer2D::Statistics Stats;
	};

	static std::unique_ptr<Renderer2DData> s_Data;

	void Renderer2D::Init()
	{
		s_Data = std::make_unique<Renderer2DData>();

		// Enable Alpha Blending
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

		// 1. Allocate CPU Buffer
		s_Data->QuadVertexBufferBase = new QuadVertex[s_Data->MaxVertices];

		// 2. Create VAO
		GLCall(glCreateVertexArrays(1, &s_Data->QuadVAO));
		GLCall(glBindVertexArray(s_Data->QuadVAO));

		// 3. Create VBO
		GLCall(glCreateBuffers(1, &s_Data->QuadVBO));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, s_Data->QuadVBO));
		GLCall(glBufferData(GL_ARRAY_BUFFER, s_Data->MaxVertices * sizeof(QuadVertex), nullptr, GL_DYNAMIC_DRAW));

		// 4. Layout
		// [NOTE] Keeping offsetof outside of GLCall is safer for some compilers, 
		// but wrapping the function call itself is fine.
		GLCall(glEnableVertexAttribArray(0));
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, Position)));

		GLCall(glEnableVertexAttribArray(1));
		GLCall(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, Color)));

		GLCall(glEnableVertexAttribArray(2));
		GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, TexCoord)));

		GLCall(glEnableVertexAttribArray(3));
		GLCall(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, TexIndex)));

		GLCall(glEnableVertexAttribArray(4));
		GLCall(glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(QuadVertex), (const void*)offsetof(QuadVertex, TilingFactor)));

		// 5. Create IBO
		uint32_t* quadIndices = new uint32_t[s_Data->MaxIndices];
		uint32_t offset = 0;
		for (uint32_t i = 0; i < s_Data->MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;
			offset += 4;
		}

		// [Safety] Store ID in struct member instead of local var so we can delete it
		GLCall(glCreateBuffers(1, &s_Data->QuadIBO));
		GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_Data->QuadIBO));
		GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, s_Data->MaxIndices * sizeof(uint32_t), quadIndices, GL_STATIC_DRAW));
		delete[] quadIndices;

		// 6. Create White Texture
		s_Data->WhiteTexture = std::make_shared<Texture2D>(1, 1);
		uint32_t whiteTextureData = 0xffffffff;
		s_Data->WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		// 7. Assign Slot 0
		s_Data->TextureSlots[0] = s_Data->WhiteTexture->GetRendererID();

		// 8. Initialize Shader
		s_Data->TextureShader = std::make_unique<Shader>("assets/shaders/Texture.glsl");
		s_Data->TextureShader->Bind();

		// 9. Upload Samplers
		int32_t samplers[32];
		for (uint32_t i = 0; i < 32; i++) samplers[i] = i;
		s_Data->TextureShader->SetUniform1iv("u_Textures", 32, samplers);
	}

	void Renderer2D::Shutdown()
	{
		// [Safety] Delete GPU resources properly
		GLCall(glDeleteVertexArrays(1, &s_Data->QuadVAO));
		GLCall(glDeleteBuffers(1, &s_Data->QuadVBO));
		GLCall(glDeleteBuffers(1, &s_Data->QuadIBO));

		delete[] s_Data->QuadVertexBufferBase;
		s_Data.reset();
	}

	void Renderer2D::BeginScene(const glm::mat4& viewProj)
	{
		s_Data->TextureShader->Bind();
		s_Data->TextureShader->SetUniformMat4f("u_ViewProjection", viewProj);
		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		Flush();
	}

	void Renderer2D::StartBatch()
	{
		s_Data->QuadIndexCount = 0;
		s_Data->QuadVertexBufferPtr = s_Data->QuadVertexBufferBase;
		s_Data->TextureSlotIndex = 1;
	}

	void Renderer2D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer2D::Flush()
	{
		if (s_Data->QuadIndexCount == 0)
			return;

		// Bind Raw IDs
		for (uint32_t i = 0; i < s_Data->TextureSlotIndex; i++)
		{
			GLCall(glBindTextureUnit(i, s_Data->TextureSlots[i]));
		}

		uint32_t dataSize = (uint32_t)((uint8_t*)s_Data->QuadVertexBufferPtr - (uint8_t*)s_Data->QuadVertexBufferBase);

		GLCall(glBindBuffer(GL_ARRAY_BUFFER, s_Data->QuadVBO));
		GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, s_Data->QuadVertexBufferBase));

		GLCall(glBindVertexArray(s_Data->QuadVAO));
		GLCall(glDrawElements(GL_TRIANGLES, s_Data->QuadIndexCount, GL_UNSIGNED_INT, nullptr));

		s_Data->Stats.DrawCalls++;
	}

	// --- 1. DrawQuad (COLOR) ---
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		if (s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();

		const float texIndex = 0.0f; // White Texture Slot
		const float tilingFactor = 1.0f;

		glm::vec3 positions[] = {
			{ position.x, position.y, 0.0f },
			{ position.x + size.x, position.y, 0.0f },
			{ position.x + size.x, position.y + size.y, 0.0f },
			{ position.x, position.y + size.y, 0.0f }
		};

		glm::vec2 texCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		for (int i = 0; i < 4; i++)
		{
			s_Data->QuadVertexBufferPtr->Position = positions[i];
			s_Data->QuadVertexBufferPtr->Color = color;
			s_Data->QuadVertexBufferPtr->TexCoord = texCoords[i];
			s_Data->QuadVertexBufferPtr->TexIndex = texIndex;
			s_Data->QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data->QuadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;
		s_Data->Stats.QuadCount++;
	}

	// --- 2. DrawQuad (SMART POINTER) ---
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		if (texture)
			DrawQuad(position, size, texture->GetRendererID(), tilingFactor, tintColor);
		else
			DrawQuad(position, size, tintColor); // Fallback safe
	}

	// --- 3. DrawQuad (RAW ID) ---
	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, uint32_t textureID, float tilingFactor, const glm::vec4& tintColor)
	{
		if (s_Data->QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f;

		// Check if texture is already in a slot
		for (uint32_t i = 1; i < s_Data->TextureSlotIndex; i++)
		{
			if (s_Data->TextureSlots[i] == textureID)
			{
				textureIndex = (float)i;
				break;
			}
		}

		// If not, add it
		if (textureIndex == 0.0f)
		{
			// [IMPORTANT] Flush if we run out of texture slots!
			if (s_Data->TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
				NextBatch();

			textureIndex = (float)s_Data->TextureSlotIndex;
			s_Data->TextureSlots[s_Data->TextureSlotIndex] = textureID;
			s_Data->TextureSlotIndex++;
		}

		glm::vec3 positions[] = {
			{ position.x, position.y, 0.0f },
			{ position.x + size.x, position.y, 0.0f },
			{ position.x + size.x, position.y + size.y, 0.0f },
			{ position.x, position.y + size.y, 0.0f }
		};

		glm::vec2 texCoords[] = { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} };

		for (int i = 0; i < 4; i++)
		{
			s_Data->QuadVertexBufferPtr->Position = positions[i];
			s_Data->QuadVertexBufferPtr->Color = tintColor;
			s_Data->QuadVertexBufferPtr->TexCoord = texCoords[i];
			s_Data->QuadVertexBufferPtr->TexIndex = textureIndex;
			s_Data->QuadVertexBufferPtr->TilingFactor = tilingFactor;
			s_Data->QuadVertexBufferPtr++;
		}

		s_Data->QuadIndexCount += 6;
		s_Data->Stats.QuadCount++;
	}

	void Renderer2D::ResetStats()
	{
		memset(&s_Data->Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return s_Data->Stats;
	}
}