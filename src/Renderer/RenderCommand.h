#pragma once

#include "Renderer/RendererAPI.h"
#include <memory>
#include <glm/glm.hpp>

namespace Avalon {

    class VertexArray;
    class Shader;
    class Material;

    /**
     * @brief Abstract base class for all render queue command packets.
     */
    class RenderCommand {
    public:
        virtual ~RenderCommand() = default;

        // Executes the command on the GPU using state deduplication tracking
        virtual void Execute(uint32_t& activeShaderID, uint32_t& activeMaterialID) = 0;

        // Generates the sort key for minimizing state changes
        virtual uint64_t GetSortKey() const = 0;
    };

    /**
     * @brief Concrete command for drawing a static model mesh.
     */
    class MeshDrawCommand : public RenderCommand {
    public:
        MeshDrawCommand(
            const std::shared_ptr<Shader>& shader,
            const std::shared_ptr<VertexArray>& vertexArray,
            const std::shared_ptr<Material>& material,
            const glm::mat4& transform,
            const glm::mat3& normalMatrix,
            uint64_t sortKey
        ) : m_Shader(shader), m_VertexArray(vertexArray), m_Material(material), 
            m_Transform(transform), m_NormalMatrix(normalMatrix), m_SortKey(sortKey) {}

        ~MeshDrawCommand() override = default;

        void Execute(uint32_t& activeShaderID, uint32_t& activeMaterialID) override;
        uint64_t GetSortKey() const override { return m_SortKey; }

        const std::shared_ptr<Shader>& GetShader() const { return m_Shader; }
        const std::shared_ptr<VertexArray>& GetVertexArray() const { return m_VertexArray; }
        const std::shared_ptr<Material>& GetMaterial() const { return m_Material; }
        const glm::mat4& GetTransform() const { return m_Transform; }
        const glm::mat3& GetNormalMatrix() const { return m_NormalMatrix; }

    private:
        std::shared_ptr<Shader> m_Shader;
        std::shared_ptr<VertexArray> m_VertexArray;
        std::shared_ptr<Material> m_Material;
        glm::mat4 m_Transform;
        glm::mat3 m_NormalMatrix;
        uint64_t m_SortKey;
    };

    /**
     * @brief RenderCommandAPI (renamed static class)
     * Provides a direct, low-level wrapper around the RendererAPI.
     */
    class RenderCommandAPI {
    public:
        static void Init() {
            s_RendererAPI->Init();
        }

        static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
            s_RendererAPI->SetViewport(x, y, width, height);
        }

        static void SetClearColor(const glm::vec4& color) {
            s_RendererAPI->SetClearColor(color);
        }

        static void Clear() {
            s_RendererAPI->Clear();
        }

        static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) {
            s_RendererAPI->DrawIndexed(vertexArray, indexCount);
        }

    private:
        static std::unique_ptr<RendererAPI> s_RendererAPI;
    };

} // namespace Avalon
