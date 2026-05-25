#include "Resource/ResourceManager.h"
#include "Shader/Shader.h"
#include "Resource/Texture.h"
#include <iostream>

namespace Avalon {

    std::unordered_map<std::string, std::shared_ptr<Shader>> ResourceManager::s_Shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture>> ResourceManager::s_Textures;

    void ResourceManager::Clear() {
        s_Shaders.clear();
        s_Textures.clear();
        std::cout << "[Resource Manager] Cleared all cached shaders and textures." << std::endl;
    }

    std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& filepath) {
        auto it = s_Shaders.find(filepath);
        if (it != s_Shaders.end()) {
            return it->second;
        }

        auto shader = std::make_shared<Shader>(filepath);
        s_Shaders[filepath] = shader;
        std::cout << "[Resource Manager] Compiled and cached Shader: " << filepath << std::endl;
        return shader;
    }

    std::shared_ptr<Texture> ResourceManager::LoadTexture(const std::string& filepath) {
        auto it = s_Textures.find(filepath);
        if (it != s_Textures.end()) {
            return it->second;
        }

        auto texture = std::make_shared<Texture>(filepath);
        s_Textures[filepath] = texture;
        std::cout << "[Resource Manager] Loaded and cached Texture: " << filepath << std::endl;
        return texture;
    }

} // namespace Avalon
