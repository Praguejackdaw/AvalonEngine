#include "Resource/Model.h"
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
        std::vector<MeshTexture> textures;

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

        // Process Materials
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            // 1. Diffuse maps
            std::vector<MeshTexture> diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            // 2. Specular maps
            std::vector<MeshTexture> specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

            // 3. Normal maps (often loaded as height maps in older Assimp formats)
            std::vector<MeshTexture> normalMaps = LoadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
            if (normalMaps.empty()) {
                normalMaps = LoadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
            }
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        }

        return Mesh(vertices, indices, textures);
    }

    std::vector<MeshTexture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName) {
        std::vector<MeshTexture> textures;
        
        for (uint32_t i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str);
            
            // Check cache to avoid reloading textures
            bool skip = false;
            for (uint32_t j = 0; j < m_TexturesLoaded.size(); j++) {
                if (std::strcmp(m_TexturesLoaded[j].TextureInstance->GetPath().c_str(), (m_Directory + "/" + str.C_Str()).c_str()) == 0) {
                    textures.push_back(m_TexturesLoaded[j]);
                    skip = true;
                    break;
                }
            }

            if (!skip) {
                // Texture hasn't been loaded before, construct and load it
                std::string fullPath = m_Directory + "/" + str.C_Str();
                auto texture = ResourceManager::LoadTexture(fullPath);
                
                MeshTexture tex;
                tex.TextureInstance = texture;
                tex.Type = typeName;
                textures.push_back(tex);
                m_TexturesLoaded.push_back(tex); // Cache it
            }
        }

        return textures;
    }

} // namespace Avalon
