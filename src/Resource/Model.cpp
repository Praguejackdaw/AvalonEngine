#include "Resource/Model.h"
#include "Resource/Material.h"
#include "Resource/Texture.h"
#include "Shader/Shader.h"
#include "Resource/ResourceManager.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <stdexcept>

namespace Avalon {

    Model::Model(const std::string& path) {
        LoadModel(path);
    }

    void Model::Draw(const std::shared_ptr<Shader>& shader) {
        for (auto& mesh : m_Meshes) {
            mesh.Draw(shader);
        }
    }

    void Model::LoadModel(const std::string& path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, 
            aiProcess_Triangulate | 
            aiProcess_GenSmoothNormals | 
            aiProcess_FlipUVs | 
            aiProcess_CalcTangentSpace
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Assimp error loading model: " << importer.GetErrorString() << std::endl;
            throw std::runtime_error("Assimp model loading failed!");
        }

        m_Directory = path.substr(0, path.find_last_of("/\\"));
        ProcessNode(scene->mRootNode, scene);
    }

    void Model::ProcessNode(aiNode* node, const aiScene* scene) {
        // Process meshes in this node
        for (uint32_t i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_Meshes.push_back(ProcessMesh(mesh, scene));
        }

        // Recursively process child nodes
        for (uint32_t i = 0; i < node->mNumChildren; i++) {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        // Process Vertices
        for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            
            // Positions
            vertex.Position.x = mesh->mVertices[i].x;
            vertex.Position.y = mesh->mVertices[i].y;
            vertex.Position.z = mesh->mVertices[i].z;

            // Normals
            if (mesh->HasNormals()) {
                vertex.Normal.x = mesh->mNormals[i].x;
                vertex.Normal.y = mesh->mNormals[i].y;
                vertex.Normal.z = mesh->mNormals[i].z;
            } else {
                vertex.Normal = glm::vec3(0.0f);
            }

            // Texture Coordinates
            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
                vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
            } else {
                vertex.TexCoords = glm::vec2(0.0f);
            }

            vertices.push_back(vertex);
        }

        // Process Indices
        for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        std::shared_ptr<Material> meshMaterial = nullptr;

        // Process Materials
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            // Load Albedo and Specular texture maps
            std::shared_ptr<Texture> albedoTex = LoadMaterialTexture(material, aiTextureType_DIFFUSE);
            std::shared_ptr<Texture> specularTex = LoadMaterialTexture(material, aiTextureType_SPECULAR);

            float shininess = 32.0f;
            material->Get(AI_MATKEY_SHININESS, shininess);

            meshMaterial = BlinnPhongMaterial::Create(albedoTex, specularTex, shininess);
        } else {
            meshMaterial = BlinnPhongMaterial::Create();
        }

        return Mesh(vertices, indices, meshMaterial);
    }

    std::shared_ptr<Texture> Model::LoadMaterialTexture(aiMaterial* mat, aiTextureType type) {
        if (mat->GetTextureCount(type) == 0) {
            return nullptr;
        }

        aiString str;
        mat->GetTexture(type, 0, &str);

        std::string fullPath = m_Directory + "/" + str.C_Str();

        // Check cache to avoid reloading textures
        for (auto& loadedTex : m_TexturesLoaded) {
            if (loadedTex->GetPath() == fullPath) {
                return loadedTex;
            }
        }

        // Texture hasn't been loaded before, construct and load it
        auto texture = ResourceManager::LoadTexture(fullPath);
        m_TexturesLoaded.push_back(texture); // Cache it
        return texture;
    }

} // namespace Avalon
