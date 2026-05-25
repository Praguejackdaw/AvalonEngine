#include "Renderer/Buffer.h"
#include <glad/glad.h>

namespace Avalon {

    // ----------------------------------------------------
    // Vertex Buffer Implementation
    // ----------------------------------------------------
    VertexBuffer::VertexBuffer(float* vertices, uint32_t size, bool dynamic) {
        // DSA: Creates buffer on GPU without binding it
        glCreateBuffers(1, &m_RendererID);
        
        // Define storage characteristics (C++17 style)
        GLbitfield flags = dynamic ? GL_DYNAMIC_STORAGE_BIT : 0;
        
        // Allocate immutable storage
        glNamedBufferStorage(m_RendererID, size, vertices, flags);
    }

    VertexBuffer::~VertexBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    void VertexBuffer::Bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void VertexBuffer::Unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::SetData(const void* data, uint32_t size) {
        // DSA: Safely update parts of the buffer without binding
        glNamedBufferSubData(m_RendererID, 0, size, data);
    }

    // ----------------------------------------------------
    // Index Buffer Implementation
    // ----------------------------------------------------
    IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count)
        : m_Count(count) {
        glCreateBuffers(1, &m_RendererID);
        // Immutable storage allocation for indices
        glNamedBufferStorage(m_RendererID, count * sizeof(uint32_t), indices, 0);
    }

    IndexBuffer::~IndexBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    void IndexBuffer::Bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    }

    void IndexBuffer::Unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

} // namespace Avalon
