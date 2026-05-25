#pragma once

#include "Resource/Mesh.h"
#include <string>
#include <vector>
#include <memory>

struct aiNode;
struct aiMesh;
struct aiScene;
struct aiMaterial;
enum aiTextureType;

namespace Avalon {

    class Shader;

    class Model {
    public:
        explicit Model(const std::string& path);
        ~Model() = default;

        void Draw(const std::shared_ptr<Shader>& shader);

        const std::vector<Mesh>& GetMeshes() const { return m_Meshes; }

    private:
        void LoadModel(const std::string& path);
        void ProcessNode(aiNode* node, const aiScene* scene);
        Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<MeshTexture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);

    private:
        std::vector<Mesh> m_Meshes;
        std::string m_Directory;
        std::vector<MeshTexture> m_TexturesLoaded; // Cache to prevent duplicate textures load
    };

} // namespace Avalon
