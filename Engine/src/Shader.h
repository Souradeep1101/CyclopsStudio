#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

#include "glm/glm.hpp"

namespace Cyclops
{
	struct ShaderProgramSource
	{
		std::string VertexSource;
		std::string FragmentSource;
	};

	/**
	 * @class Shader
	 * @brief Wrapper for OpenGL Shader Programs.
	 * * @details Handles loading source code from files, compiling vertex/fragment stages,
	 * linking the program, and managing Uniform caching for performance.
	 */
	class Shader
	{
	private:
		std::string m_FilePath;
		uint32_t m_RendererID;
		
		/// @brief Cache to avoid calling glGetUniformLocation every frame (slow).
		std::unordered_map<std::string, int32_t> m_UniformLocationCache;
	public:
		/**
		 * @brief Loads and compiles a shader from a single file.
		 * @param filepath Path to the shader file (must contain #shader vertex and #shader fragment).
		 */
		Shader(const std::string& filepath);
		~Shader();

		// Safety: Shaders are unique GPU resources.
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		void Bind() const;
		void UnBind() const;

		// --- Uniform Setters ---
		// These functions upload data to the GPU for the currently bound shader.

		void SetUniform1i(const std::string& name, int32_t value);
		void SetUniform1f(const std::string& name, float value);
		void SetUniform3f(const std::string& name, const glm::vec3& value);
		void SetUniform4f(const std::string& name, const glm::vec4& value);
		void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
	private:
		ShaderProgramSource ParseShader(const std::string& filepath);
		uint32_t CompileShader(uint32_t type, const std::string& source);
		int32_t GetUniformLocation(const std::string& name);
		uint32_t CreateShader(const std::string& vertexShader, const std::string& fragmentShader);
	};

}
