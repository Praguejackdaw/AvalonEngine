#include "Shader/Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace Avalon {

    uint32_t Shader::s_NextShaderID = 1;

    static GLenum ShaderTypeFromString(const std::string& type) {
        if (type == "vertex") {
            return GL_VERTEX_SHADER;
        }
        if (type == "fragment" || type == "pixel") {
            return GL_FRAGMENT_SHADER;
        }
        throw std::runtime_error("Unknown shader type: " + type);
    }

    Shader::Shader(const std::string& filepath)
        : m_FilePath(filepath) {
        m_ShaderID = s_NextShaderID++;

        // Read file name from path
        size_t lastSlash = filepath.find_last_of("/\\");
        lastSlash = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
        size_t lastDot = filepath.rfind('.');
        size_t count = (lastDot == std::string::npos) ? filepath.size() - lastSlash : lastDot - lastSlash;
        m_Name = filepath.substr(lastSlash, count);

        std::string source = ReadFile(filepath);
        auto shaderSources = PreProcess(source);
        Compile(shaderSources);
    }

    Shader::Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
        : m_Name(name) {
        m_ShaderID = s_NextShaderID++;

        std::unordered_map<unsigned int, std::string> sources;
        sources[GL_VERTEX_SHADER] = vertexSrc;
        sources[GL_FRAGMENT_SHADER] = fragmentSrc;
        Compile(sources);
    }

    Shader::~Shader() {
        glDeleteProgram(m_RendererID);
    }

    std::string Shader::ReadFile(const std::string& filepath) {
        std::string result;
        std::ifstream in(filepath, std::ios::in | std::ios::binary);
        if (in) {
            in.seekg(0, std::ios::end);
            size_t size = in.tellg();
            if (size != -1) {
                result.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&result[0], size);
                in.close();
            } else {
                throw std::runtime_error("Could not read from file: " + filepath);
            }
        } else {
            throw std::runtime_error("Failed to open shader file: " + filepath);
        }
        return result;
    }

    std::unordered_map<unsigned int, std::string> Shader::PreProcess(const std::string& source) {
        std::unordered_map<unsigned int, std::string> shaderSources;

        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = source.find(typeToken, 0);
        while (pos != std::string::npos) {
            size_t eol = source.find_first_of("\r\n", pos);
            if (eol == std::string::npos) {
                throw std::runtime_error("Shader syntax error!");
            }
            size_t begin = pos + typeTokenLength + 1;
            std::string type = source.substr(begin, eol - begin);
            // Trim whitespace
            type.erase(0, type.find_first_not_of(" \t"));
            type.erase(type.find_last_not_of(" \t") + 1);

            size_t nextLinePos = source.find_first_not_of("\r\n", eol);
            if (nextLinePos == std::string::npos) {
                throw std::runtime_error("Shader syntax error: empty shader after type declaration!");
            }
            pos = source.find(typeToken, nextLinePos);

            shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
        }

        return shaderSources;
    }

    void Shader::Compile(const std::unordered_map<unsigned int, std::string>& shaderSources) {
        GLuint program = glCreateProgram();
        std::vector<GLuint> shaderIDs;
        shaderIDs.reserve(shaderSources.size());

        for (auto& kv : shaderSources) {
            GLenum type = kv.first;
            const std::string& source = kv.second;

            GLuint shader = glCreateShader(type);
            const GLchar* sourceCStr = source.c_str();
            glShaderSource(shader, 1, &sourceCStr, 0);
            glCompileShader(shader);

            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE) {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

                glDeleteShader(shader);
                for (auto id : shaderIDs) {
                    glDeleteShader(id);
                }
                glDeleteProgram(program);

                std::cerr << "Shader compile failure (" << m_Name << "): " << &infoLog[0] << std::endl;
                throw std::runtime_error("Shader compilation failed!");
            }

            glAttachShader(program, shader);
            shaderIDs.push_back(shader);
        }

        glLinkProgram(program);

        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE) {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            for (auto id : shaderIDs) {
                glDeleteShader(id);
            }
            glDeleteProgram(program);

            std::cerr << "Program link failure (" << m_Name << "): " << &infoLog[0] << std::endl;
            throw std::runtime_error("Program linkage failed!");
        }

        // Detach shaders after link
        for (auto id : shaderIDs) {
            glDetachShader(program, id);
            glDeleteShader(id);
        }

        m_RendererID = program;

        // DSA: Set static texture sampler binding points once upon initialization to avoid redundant updates
        SetInt("u_Material.AlbedoMap", 0);
        SetInt("u_Material.MetallicRoughnessMap", 1);
        SetInt("u_Material.NormalMap", 2);
        SetInt("u_Material.AOMap", 3);
        SetInt("u_ShadowMap", 4);
    }

    void Shader::Bind() const {
        glUseProgram(m_RendererID);
    }

    void Shader::Unbind() const {
        glUseProgram(0);
    }

    int Shader::GetUniformLocation(const std::string& name) {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
            return m_UniformLocationCache[name];
        }

        int location = glGetUniformLocation(m_RendererID, name.c_str());
        m_UniformLocationCache[name] = location;
        return location;
    }

    // ----------------------------------------------------
    // DSA Uniform Setters
    // ----------------------------------------------------
    void Shader::SetInt(const std::string& name, int value) {
        int location = GetUniformLocation(name);
        if (location != -1) {
            glProgramUniform1i(m_RendererID, location, value);
        }
    }

    void Shader::SetIntArray(const std::string& name, int* values, uint32_t count) {
        int location = GetUniformLocation(name);
        if (location != -1) {
            glProgramUniform1iv(m_RendererID, location, static_cast<GLsizei>(count), values);
        }
    }

    void Shader::SetFloat(const std::string& name, float value) {
        int location = GetUniformLocation(name);
        if (location != -1) {
            glProgramUniform1f(m_RendererID, location, value);
        }
    }

    void Shader::SetFloat2(const std::string& name, const glm::vec2& value) {
        int location = GetUniformLocation(name);
        if (location != -1) {
            glProgramUniform2fv(m_RendererID, location, 1, glm::value_ptr(value));
        }
    }

    void Shader::SetFloat3(const std::string& name, const glm::vec3& value) {
        int location = GetUniformLocation(name);
        if (location != -1) {
            glProgramUniform3fv(m_RendererID, location, 1, glm::value_ptr(value));
        }
    }

    void Shader::SetFloat4(const std::string& name, const glm::vec4& value) {
        int location = GetUniformLocation(name);
        if (location != -1) {
            glProgramUniform4fv(m_RendererID, location, 1, glm::value_ptr(value));
        }
    }

    void Shader::SetMat3(const std::string& name, const glm::mat3& value) {
        int location = GetUniformLocation(name);
        if (location != -1) {
            glProgramUniformMatrix3fv(m_RendererID, location, 1, GL_FALSE, glm::value_ptr(value));
        }
    }

    void Shader::SetMat4(const std::string& name, const glm::mat4& value) {
        int location = GetUniformLocation(name);
        if (location != -1) {
            glProgramUniformMatrix4fv(m_RendererID, location, 1, GL_FALSE, glm::value_ptr(value));
        }
    }

} // namespace Avalon
