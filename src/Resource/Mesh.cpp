#include "Resource/Mesh.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Buffer.h"
#include "Renderer/RenderCommand.h"
#include "Shader/Shader.h"
#include "Resource/Material.h"
#include <iostream>

namespace Avalon {

    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::shared_ptr<Material>& material)
        : m_Vertices(vertices), m_Indices(indices), m_Material(material) {
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
        if (m_Material) {
            m_Material->Bind(shader);
        }
        // Draw mesh elements
        RenderCommandAPI::DrawIndexed(m_VertexArray);
    }

} // namespace Avalon
