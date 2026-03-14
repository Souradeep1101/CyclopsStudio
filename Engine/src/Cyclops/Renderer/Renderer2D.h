#pragma once

#include "Cyclops/Renderer/Texture.h"
#include "Cyclops/Core/Base.h"

#include <glm/glm.hpp>
#include <cstdint>
#include <memory>

namespace Cyclops
{
	class CYCLOPS_API Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const glm::mat4& viewProj);
		static void EndScene();
		static void Flush();

		// --- Primitives ---

		// 1. Color Quad (Uses White Texture automatically)
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);

		// 2. Texture Quad (Uses Texture Slots)
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		// 3. [NEW] Raw Texture ID (For Framebuffers/Generated Textures)
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, uint32_t textureID, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		
		// Statistics
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};

		static void ResetStats();
		static Statistics GetStats();

	private:
		static void StartBatch();
		static void NextBatch(); // <--- Added this declaration
	};
}