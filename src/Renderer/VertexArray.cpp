#include "Renderer/VertexArray.h"
#include <glad/glad.h>
#include <stdexcept>

namespace Avalon {

    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:    return GL_FLOAT;
            case ShaderDataType::Float2:   return GL_FLOAT;
            case ShaderDataType::Float3:   return GL_FLOAT;
            case ShaderDataType::Float4:   return GL_FLOAT;
            case ShaderDataType::Mat3:     return GL_FLOAT;
            case ShaderDataType::Mat4:     return GL_FLOAT;
            case ShaderDataType::Int:      return GL_INT;
            case ShaderDataType::Int2:     return GL_INT;
            case ShaderDataType::Int3:     return GL_INT;
            case ShaderDataType::Int4:     return GL_INT;
            case ShaderDataType::Bool:     return GL_BOOL;
            case ShaderDataType::None:     break;
        }
        throw std::runtime_error("Unknown ShaderDataType!");
    }

    VertexArray::VertexArray() {
        // DSA: Creates VAO handle directly without binding state changes
        glCreateVertexArrays(1, &m_RendererID);
    }

    VertexArray::~VertexArray() {
        glDeleteVertexArrays(1, &m_RendererID);
    }

    // Move Constructor
    VertexArray::VertexArray(VertexArray&& other) noexcept
        : m_RendererID(other.m_RendererID),
          m_VertexBufferIndex(other.m_VertexBufferIndex),
          m_AttribIndex(other.m_AttribIndex),
          m_VertexBuffers(std::move(other.m_VertexBuffers)),
          m_IndexBuffer(std::move(other.m_IndexBuffer)) {
        other.m_RendererID = 0;
        other.m_VertexBufferIndex = 0;
        other.m_AttribIndex = 0;
    }

    // Move Assignment
    VertexArray& VertexArray::operator=(VertexArray&& other) noexcept {
        if (this != &other) {
            glDeleteVertexArrays(1, &m_RendererID);

            m_RendererID = other.m_RendererID;
            m_VertexBufferIndex = other.m_VertexBufferIndex;
            m_AttribIndex = other.m_AttribIndex;
            m_VertexBuffers = std::move(other.m_VertexBuffers);
            m_IndexBuffer = std::move(other.m_IndexBuffer);

            other.m_RendererID = 0;
            other.m_VertexBufferIndex = 0;
            other.m_AttribIndex = 0;
        }
        return *this;
    }

    void VertexArray::Bind() const {
        glBindVertexArray(m_RendererID);
    }

    void VertexArray::Unbind() const {
        glBindVertexArray(0);
    }

    void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) {
        if (vertexBuffer->GetLayout().GetElements().empty()) {
            throw std::runtime_error("VertexBuffer has no layout defined!");
        }

        uint32_t bindingIndex = m_VertexBufferIndex;

        // DSA: Bind VertexBuffer to specific binding slot on the VAO without binding either object globally
        glVertexArrayVertexBuffer(
            m_RendererID,
            bindingIndex,
            vertexBuffer->GetRendererID(),
            0,
            vertexBuffer->GetLayout().GetStride()
        );

        const auto& layout = vertexBuffer->GetLayout();

        for (const auto& element : layout) {
            switch (element.Type) {
                case ShaderDataType::Float:
                case ShaderDataType::Float2:
                case ShaderDataType::Float3:
                case ShaderDataType::Float4:
                case ShaderDataType::Int:
                case ShaderDataType::Int2:
                case ShaderDataType::Int3:
                case ShaderDataType::Int4:
                case ShaderDataType::Bool: {
                    // DSA: Enable the vertex attribute location index
                    glEnableVertexArrayAttrib(m_RendererID, m_AttribIndex);

                    // DSA: Specify format of the vertex attribute
                    glVertexArrayAttribFormat(
                        m_RendererID,
                        m_AttribIndex,
                        element.GetComponentCount(),
                        ShaderDataTypeToOpenGLBaseType(element.Type),
                        element.Normalized ? GL_TRUE : GL_FALSE,
                        static_cast<GLuint>(element.Offset)
                    );

                    // DSA: Bind attribute index to the VertexBuffer slot index
                    glVertexArrayAttribBinding(m_RendererID, m_AttribIndex, bindingIndex);
                    m_AttribIndex++;
                    break;
                }
                case ShaderDataType::Mat3:
                case ShaderDataType::Mat4: {
                    // Matrices take multiple layout locations (columns)
                    uint8_t count = element.GetComponentCount();
                    for (uint8_t i = 0; i < count; i++) {
                        glEnableVertexArrayAttrib(m_RendererID, m_AttribIndex);
                        
                        glVertexArrayAttribFormat(
                            m_RendererID,
                            m_AttribIndex,
                            count,
                            ShaderDataTypeToOpenGLBaseType(element.Type),
                            element.Normalized ? GL_TRUE : GL_FALSE,
                            static_cast<GLuint>(element.Offset + i * count * sizeof(float))
                        );

                        glVertexArrayAttribBinding(m_RendererID, m_AttribIndex, bindingIndex);
                        
                        // Inform OpenGL this column behaves as part of an instance or custom index step if needed, but for standard formats:
                        glVertexArrayBindingDivisor(m_RendererID, bindingIndex, 0); 
                        m_AttribIndex++;
                    }
                    break;
                }
                default:
                    throw std::runtime_error("Unknown ShaderDataType!");
            }
        }

        m_VertexBuffers.push_back(vertexBuffer);
        m_VertexBufferIndex++;
    }

    void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) {
        // DSA: Associate Element (Index) buffer directly with the VAO without binding either object globally
        glVertexArrayElementBuffer(m_RendererID, indexBuffer->GetRendererID());
        m_IndexBuffer = indexBuffer;
    }

} // namespace Avalon
