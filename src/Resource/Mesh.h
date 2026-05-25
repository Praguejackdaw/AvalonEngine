#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

namespace Avalon {

    class VertexArray;
    class Shader;
    class Texture;

    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    class Material;

    class Mesh {
    public:
        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::shared_ptr<Material>& material);
        ~Mesh() = default;

        void Draw(const std::shared_ptr<Shader>& shader);

        const std::shared_ptr<VertexArray>& GetVertexArray() const { return m_VertexArray; }
        const std::shared_ptr<Material>& GetMaterial() const { return m_Material; }

    private:
        void SetupMesh();

    private:
        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::shared_ptr<Material> m_Material;

        std::shared_ptr<VertexArray> m_VertexArray;
    };

} // namespace Avalon
