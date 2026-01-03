#include "Shader.h"
#include <glad/glad.h>
#include "OpenGLDebug.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

namespace Cyclops
{
    Shader::Shader(const std::string& filepath)
        : m_FilePath{ filepath }, m_RendererID{ 0 }
    {
        ShaderProgramSource source = ParseShader(filepath);
        /*std::cout << "Vertex Shader:" << source.VertexSource << std::endl;
        std::cout << "Fragment Shader:" << source.FragmentSource << std::endl;*/
        m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
    }

    Shader::~Shader()
    {
        GLCall(glDeleteProgram(m_RendererID));
    }

    // Set Int 1
    void Shader::SetUniform1i(const std::string& name, int32_t value)
    {
        GLCall(glUniform1i(GetUniformLocation(name), value));
    }

    // Set Float 1
    void Shader::SetUniform1f(const std::string& name, float value)
    {
        GLCall(glUniform1f(GetUniformLocation(name), value));
    }

    // Set Float 3
    void Shader::SetUniform3f(const std::string& name, const glm::vec3& value)
    {
        GLCall(glUniform3f(GetUniformLocation(name), value.x, value.y, value.z));
    }

    // Set Float 4
    void Shader::SetUniform4f(const std::string& name, const glm::vec4& value)
    {
        GLCall(glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w));
    }

    // Set Float Matrix 4
    void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
    {
        GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
    }

    // Bind
    void Shader::Bind() const
    {
        GLCall(glUseProgram(m_RendererID));
    }

    // Unbind
    void Shader::UnBind() const
    {
        GLCall(glUseProgram(0));
    }

    int32_t Shader::GetUniformLocation(const std::string& name)
    {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
            return m_UniformLocationCache[name];

        GLCall(uint32_t location = glGetUniformLocation(m_RendererID, name.c_str()));
        if (location == -1)
            std::cout << "warning: uniform '" << name << "' doesn't exist!" << std::endl;

        m_UniformLocationCache[name] = location;
        return location;
    }

    ShaderProgramSource Shader::ParseShader(const std::string& filepath)
    {
        std::ifstream stream(filepath);

        enum class ShaderType
        {
            NONE = -1, VERTEX = 0, FRAGMENT = 1
        };

        ShaderType type = ShaderType::NONE;
        std::string line;
        std::stringstream ss[2];

        while (getline(stream, line))
        {
            if (line.find("#shader") != std::string::npos)
            {
                if (line.find("vertex") != std::string::npos)
                {
                    type = ShaderType::VERTEX;
                }
                else if (line.find("fragment") != std::string::npos)
                {
                    type = ShaderType::FRAGMENT;
                }
            }
            else
            {
                ss[(int32_t)type] << line << '\n';
            }
        }

        return { ss[0].str(), ss[1].str() };
    }

    uint32_t Shader::CompileShader(uint32_t type, const std::string& source)
    {
        GLCall(uint32_t id = glCreateShader(type));
        const char* src = source.c_str();
        GLCall(glShaderSource(id, 1, &src, nullptr));
        GLCall(glCompileShader(id));

        int32_t result;
        GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
        if (result == GL_FALSE)
        {
            int32_t length;
            GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
            char* message = (char*)alloca(length * sizeof(char));
            GLCall(glGetShaderInfoLog(id, length, &length, message));
            std::cout << "Failed to compile " <<
                (type == GL_VERTEX_SHADER ? "vertex" : "fragment") <<
                "shader!" << std::endl;
            std::cout << message << std::endl;
            GLCall(glDeleteShader(id));
            return 0;
        }

        return id;
    }

    uint32_t Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
    {
        GLCall(uint32_t program = glCreateProgram());
        uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
        uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

        GLCall(glAttachShader(program, vs));
        GLCall(glAttachShader(program, fs));
        GLCall(glLinkProgram(program));
        GLCall(glValidateProgram(program));

        GLCall(glDeleteShader(vs));
        GLCall(glDeleteShader(fs));

        return program;
    }
}