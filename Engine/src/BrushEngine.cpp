#include "BrushEngine.h"
#include "OpenGLDebug.h"
#include <algorithm>
#include <filesystem>
#include "Debug/Instrumentor.h"

namespace Cyclops
{
	BrushEngine::BrushEngine()
	{

	}

	BrushEngine::~BrushEngine()
	{
		GLCall(glDeleteVertexArrays(1, &m_VAO));
		GLCall(glDeleteBuffers(1, &m_VBO));
	}

	void BrushEngine::Init()
	{
		// 1. Load the Shader
		// Make sure you created the file at assets/shaders/Brush.shader
		std::string relativePath = "Engine/assets/shaders/Brush.shader";
		m_Shader =  std::make_unique<Shader>(relativePath);

		// --- Boilerplate Code ---
		//if (std::filesystem::exists(relativePath)) {
		//	std::cout << "[SUCCESS] Found shader at: " << relativePath << std::endl;
		//	m_Shader = new Shader(relativePath);
		//}
		//else {
		//	// 2. If failed, print exactly where we looked
		//	std::cout << "[ERROR] Could not find shader!" << std::endl;
		//	std::cout << "   Looking for: " << relativePath << std::endl;
		//	std::cout << "   Inside CWD : " << std::filesystem::current_path() << std::endl;
		//	std::cout << "   Full Absolute Path checked: " << std::filesystem::absolute(relativePath) << std::endl;

		//	// 3. HARDCODED FALLBACK (To prove the code works)
		//	// Replace this path with the REAL path on your disk temporarily to get unblocked
		//	std::string absolutePath = "D:/Projects/Visual Studio/CyclopsStudio/Engine/assets/shaders/Brush.shader";
		//	std::cout << "   Attempting Absolute Path Fallback: " << absolutePath << std::endl;
		//	m_Shader = new Shader(absolutePath);
		//}
		// ----

		// 2. Create the Brush Geometry (Unit Quad)
		// Centered at 0,0, Size 1x1
		float vertices[] = {
			 -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
			  0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
			  0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
			 -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
		};

		GLCall(glGenVertexArrays(1, &m_VAO));
		GLCall(glBindVertexArray(m_VAO));

		GLCall(glGenBuffers(1, &m_VBO));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_VBO));
		GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

		GLCall(glEnableVertexAttribArray(0));
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));

		GLCall(glEnableVertexAttribArray(1));
		GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));

		// Unbind to keep state clean
		GLCall(glBindVertexArray(0));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	void BrushEngine::BeginStroke(Framebuffer* target, float x, float y)
	{
		m_CurrentTarget = target;
		m_LastPos = { x, y };

		// Paint the initial dot immediately so single clicks work
		PaintQuad(x, y);
	}

	void BrushEngine::ContinueStroke(float x, float y, float pressure)
	{
		if (!m_CurrentTarget) return;

		glm::vec2 currentPos = { x, y };

		// 1. Calculate distance moved since last frame
		float distance = glm::distance(m_LastPos, currentPos);

		// 2. Determine step size (Spacing)
		// Spacing is a percentage of brush size. 
		// 0.1 spacing on a 50px brush = 5px gap between dots.
		float stepSize = std::max(1.0f, m_Settings.Size * m_Settings.Spacing);

		// 3. Interpolation Loop (The "Splatting" Logic)
		// If we moved 50px and stepSize is 5px, we draw 10 dots.
		if (distance >= stepSize)
		{
			float steps = distance / stepSize;

			for (float i = 1.0f; i <= steps; i++)
			{
				// Linear Interpolation (Lerp)
				float t = i / steps;
				glm::vec2 drawPos = glm::mix(m_LastPos, currentPos, t);

				PaintQuad(drawPos.x, drawPos.y);
			}

			// Update last pos to the end of the stroke segment
			m_LastPos = currentPos;
		}
	}

	void BrushEngine::EndStroke()
	{
		m_CurrentTarget = nullptr;
	}

	std::vector<glm::vec2> BrushEngine::CalculateInterpolation(glm::vec2 start, glm::vec2 end, float spacing, float brushSize)
	{
		std::vector<glm::vec2> points;

		// 1. Calculate distance
		float distance = glm::distance(start, end);

		// 2. Determine step size (Spacing)
		float stepSize = std::max(1.0f, brushSize * spacing);

		// 3. Interpolation Logic
		if (distance >= stepSize)
		{
			float steps = distance / stepSize;

			for (float i = 1.0f; i <= steps; i++)
			{
				float t = i / steps;
				glm::vec2 drawPos = glm::mix(start, end, t);
				points.push_back(drawPos);
			}
		}

		return points;
	}

	void BrushEngine::PaintQuad(float x, float y)
	{
		CYCLOPS_PROFILE_FUNCTION();

		// 1. Setup Render State
		m_CurrentTarget->Bind();

		// Essential: Additive blending (or Source Alpha) for smooth edges
		GLCall(glEnable(GL_BLEND));
		GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		// glBlendEquation(GL_FUNC_ADD); // Default

		// 2. Setup Camera
		// Note: In a real engine, cache this projection matrix. Don't recalculate every dot.
		float width = (float)m_CurrentTarget->GetWidth();
		float height = (float)m_CurrentTarget->GetHeight();
		glm::mat4 projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

		// 3. Setup Transform
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(x, y, 0.0f));
		model = glm::scale(model, glm::vec3(m_Settings.Size, m_Settings.Size, 1.0f));

		// 4. Draw
		m_Shader->Bind();
		m_Shader->SetUniformMat4f("u_MVP", projection * model);
		m_Shader->SetUniform4f("u_Color", m_Settings.Color);

		GLCall(glBindVertexArray(m_VAO));
		GLCall(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));
		GLCall(glBindVertexArray(0)); // Unbind VAO

		m_CurrentTarget->Unbind();
	}

}