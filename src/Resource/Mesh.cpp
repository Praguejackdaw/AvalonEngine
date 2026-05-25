#include "Resource/Mesh.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Buffer.h"
#include "Renderer/RenderCommand.h"
#include "Shader/Shader.h"
#include "Resource/Texture.h"
#include <iostream>

namespace Avalon {

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<MeshTexture>& textures)
        : m_Vertices(vertices), m_Indices(indices), m_Textures(textures) {
        SetupMesh();
    }

    void Mesh::SetupMesh() {
        m_VertexArray = std::make_shared<VertexArray>();

        // Create VBO with vertex data (static draw storage since meshes are typically static geometry)
        auto vertexBuffer = std::make_shared<VertexBuffer>(
            reinterpret_cast<float*>(m_Vertices.data()),
            static_cast<uint32_t>(m_Vertices.size() * sizeof(Vertex)),
            false
        );

        // Setup Buffer layout matching Vertex struct exactly
        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal"   },
            { ShaderDataType::Float2, "a_TexCoords" }
        });

        // Add to Vertex Array (configures attributes using our DSA routines under-the-hood)
        m_VertexArray->AddVertexBuffer(vertexBuffer);

        // Create and register index buffer
        auto indexBuffer = std::make_shared<IndexBuffer>(m_Indices.data(), static_cast<uint32_t>(m_Indices.size()));
        m_VertexArray->SetIndexBuffer(indexBuffer);
    }

    void Mesh::Draw(const std::shared_ptr<Shader>& shader) {
        uint32_t diffuseNr  = 1;
        uint32_t specularNr = 1;
        uint32_t normalNr   = 1;

        // Bind all associated textures to their respective texture unit slots
        for (uint32_t i = 0; i < m_Textures.size(); i++) {
            m_Textures[i].TextureInstance->Bind(i);

            std::string number;
            std::string name = m_Textures[i].Type;
            if (name == "texture_diffuse") {
                number = std::to_string(diffuseNr++);
            } else if (name == "texture_specular") {
                number = std::to_string(specularNr++);
            } else if (name == "texture_normal") {
                number = std::to_string(normalNr++);
            }

            // DSA: Directly updates shader sampler unit index
            shader->SetInt(name + number, static_cast<int>(i));
        }

        // Draw mesh elements
        RenderCommand::DrawIndexed(m_VertexArray);
    }

} // namespace Avalon
