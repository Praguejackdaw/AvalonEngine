#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace Avalon {

    class Shader;
    class Texture;

    class ResourceManager {
    public:
        // Disable instantiation
        ResourceManager() = delete;

        // Clear all loaded resources on engine shutdown
        static void Clear();

        // High-level safe asset caching loaders
        static std::shared_ptr<Shader> LoadShader(const std::string& filepath);
        static std::shared_ptr<Texture> LoadTexture(const std::string& filepath);

    private:
        static std::unordered_map<std::string, std::shared_ptr<Shader>> s_Shaders;
        static std::unordered_map<std::string, std::shared_ptr<Texture>> s_Textures;
    };

} // namespace Avalon
