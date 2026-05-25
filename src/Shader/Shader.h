#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Avalon {

    class Shader {
    public:
        // Load shader from a single file containing both Vertex and Fragment sources (separated by tags)
        explicit Shader(const std::string& filepath);
        
        // Construct shader from direct source strings
        Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;

        void Bind() const;
        void Unbind() const;

        // Modern DSA Uniform Setters - updates GPU variables without binding the shader
        void SetInt(const std::string& name, int value);
        void SetIntArray(const std::string& name, int* values, uint32_t count);
        void SetFloat(const std::string& name, float value);
        void SetFloat2(const std::string& name, const glm::vec2& value);
        void SetFloat3(const std::string& name, const glm::vec3& value);
        void SetFloat4(const std::string& name, const glm::vec4& value);
        void SetMat4(const std::string& name, const glm::mat4& value);

        const std::string& GetName() const { return m_Name; }
        uint32_t GetRendererID() const { return m_RendererID; }

    private:
        std::string ReadFile(const std::string& filepath);
        std::unordered_map<unsigned int, std::string> PreProcess(const std::string& source);
        void Compile(const std::unordered_map<unsigned int, std::string>& shaderSources);
        int GetUniformLocation(const std::string& name);

    private:
        uint32_t m_RendererID = 0;
        std::string m_Name;
        std::string m_FilePath;
        std::unordered_map<std::string, int> m_UniformLocationCache;
    };

} // namespace Avalon
