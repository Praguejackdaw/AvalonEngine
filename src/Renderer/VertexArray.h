#pragma once

#include "Renderer/Buffer.h"
#include <memory>
#include <vector>

namespace Avalon {

    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        // Disable copy semantics to prevent double deletion of VAO m_RendererID
        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;

        // Enable move semantics for transfer of ownership
        VertexArray(VertexArray&& other) noexcept;
        VertexArray& operator=(VertexArray&& other) noexcept;

        // Bind/Unbind are only used for drawing (glDrawElements / glDrawArrays)
        void Bind() const;
        void Unbind() const;

        // Modern DSA additions
        // Associates a Vertex Buffer with a specific binding index on this VAO
        void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer);
        
        // Associates an Index Buffer (Element Buffer) directly with this VAO
        void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

        // Getters for bound buffers
        const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
        const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }

        uint32_t GetRendererID() const { return m_RendererID; }

    private:
        uint32_t m_RendererID = 0;
        uint32_t m_VertexBufferIndex = 0; // Tracks binding slots (bindingIndex) for DSA
        uint32_t m_AttribIndex = 0;        // Tracks the attribute layout location index for DSA
        std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
        std::shared_ptr<IndexBuffer> m_IndexBuffer;
    };

} // namespace Avalon
