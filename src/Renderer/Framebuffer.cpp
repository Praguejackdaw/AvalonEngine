#include "Renderer/Framebuffer.h"
#include <glad/glad.h>
#include <iostream>
#include <stdexcept>

namespace Avalon {

    static const uint32_t s_MaxFramebufferSize = 8192;

    namespace Utils {
        
        static bool IsDepthFormat(FramebufferTextureFormat format) {
            switch (format) {
                case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
                case FramebufferTextureFormat::DEPTH_COMPONENT: return true;
                default: return false;
            }
            return false;
        }

        static GLenum GLTextureFormatFromFramebufferFormat(FramebufferTextureFormat format) {
            switch (format) {
                case FramebufferTextureFormat::RGBA8: return GL_RGBA8;
                case FramebufferTextureFormat::DEPTH24STENCIL8: return GL_DEPTH24_STENCIL8;
                case FramebufferTextureFormat::DEPTH_COMPONENT: return GL_DEPTH_COMPONENT24;
                default: return 0;
            }
            return 0;
        }
    }

    Framebuffer::Framebuffer(const FramebufferSpecification& spec)
        : m_Specification(spec) {
        
        for (auto spec : m_Specification.Attachments.Attachments) {
            if (!Utils::IsDepthFormat(spec.TextureFormat)) {
                m_ColorAttachmentSpecs.push_back(spec);
            } else {
                m_DepthAttachmentSpec = spec;
            }
        }

        Invalidate();
    }

    Framebuffer::~Framebuffer() {
        Cleanup();
    }

    void Framebuffer::Cleanup() {
        if (m_RendererID) {
            glDeleteFramebuffers(1, &m_RendererID);
            
            if (!m_ColorAttachments.empty()) {
                glDeleteTextures(static_cast<GLsizei>(m_ColorAttachments.size()), m_ColorAttachments.data());
                m_ColorAttachments.clear();
            }

            if (m_DepthAttachment) {
                glDeleteTextures(1, &m_DepthAttachment);
                m_DepthAttachment = 0;
            }
            m_RendererID = 0;
        }
    }

    void Framebuffer::Invalidate() {
        if (m_RendererID) {
            Cleanup();
        }

        // OpenGL 4.5 DSA - Create Framebuffer Object directly
        glCreateFramebuffers(1, &m_RendererID);

        // Allocating Color Attachments (using DSA textures)
        if (!m_ColorAttachmentSpecs.empty()) {
            m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
            glCreateTextures(GL_TEXTURE_2D, static_cast<GLsizei>(m_ColorAttachments.size()), m_ColorAttachments.data());

            for (size_t i = 0; i < m_ColorAttachments.size(); i++) {
                uint32_t textureId = m_ColorAttachments[i];
                GLenum internalFormat = Utils::GLTextureFormatFromFramebufferFormat(m_ColorAttachmentSpecs[i].TextureFormat);

                // Immutable storage allocation (DSA)
                glTextureStorage2D(textureId, 1, internalFormat, m_Specification.Width, m_Specification.Height);

                // Set texture parameters without binding (DSA)
                glTextureParameteri(textureId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(textureId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(textureId, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTextureParameteri(textureId, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                // Attach to FBO (DSA)
                glNamedFramebufferTexture(m_RendererID, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i), textureId, 0);
            }
        }

        // Allocating Depth Attachment (using DSA texture)
        if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None) {
            glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
            GLenum internalFormat = Utils::GLTextureFormatFromFramebufferFormat(m_DepthAttachmentSpec.TextureFormat);

            // Immutable storage allocation (DSA)
            glTextureStorage2D(m_DepthAttachment, 1, internalFormat, m_Specification.Width, m_Specification.Height);

            // Set texture parameters without binding (DSA)
            glTextureParameteri(m_DepthAttachment, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(m_DepthAttachment, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTextureParameteri(m_DepthAttachment, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(m_DepthAttachment, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Attach to FBO (DSA)
            GLenum attachmentPoint = GL_DEPTH_STENCIL_ATTACHMENT;
            if (m_DepthAttachmentSpec.TextureFormat == FramebufferTextureFormat::DEPTH_COMPONENT) {
                attachmentPoint = GL_DEPTH_ATTACHMENT;
            }
            glNamedFramebufferTexture(m_RendererID, attachmentPoint, m_DepthAttachment, 0);
        }

        // Configure Draw Buffers if we have multiple color attachments or depth-only
        if (m_ColorAttachments.size() > 1) {
            std::vector<GLenum> buffers;
            for (size_t i = 0; i < m_ColorAttachments.size(); i++) {
                buffers.push_back(GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i));
            }
            glNamedFramebufferDrawBuffers(m_RendererID, static_cast<GLsizei>(buffers.size()), buffers.data());
        } else if (m_ColorAttachments.empty()) {
            // Depth-only Framebuffer (e.g. Shadow Mapping)
            glNamedFramebufferDrawBuffer(m_RendererID, GL_NONE);
            glNamedFramebufferReadBuffer(m_RendererID, GL_NONE);
        }

        // Verify status
        if (glCheckNamedFramebufferStatus(m_RendererID, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Framebuffer creation failed: FBO status is incomplete!");
        }
    }

    void Framebuffer::Bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glViewport(0, 0, m_Specification.Width, m_Specification.Height);
    }

    void Framebuffer::Unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::Resize(uint32_t width, uint32_t height) {
        if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize) {
            std::cerr << "[Framebuffer Warning] Attempted to resize framebuffer to invalid size: (" << width << ", " << height << ")" << std::endl;
            return;
        }

        m_Specification.Width = width;
        m_Specification.Height = height;
        Invalidate();
    }

} // namespace Avalon
