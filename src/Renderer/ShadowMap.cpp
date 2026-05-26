#include "Renderer/ShadowMap.h"
#include <glad/glad.h>
#include <iostream>
#include <stdexcept>

namespace Avalon {

    std::unique_ptr<ShadowMap> ShadowMap::Create(uint32_t width, uint32_t height, ShadowType type) {
        return std::make_unique<ShadowMap>(width, height, type);
    }

    ShadowMap::ShadowMap(uint32_t width, uint32_t height, ShadowType type)
        : m_Width(width), m_Height(height), m_Type(type) {
        Invalidate();
    }

    ShadowMap::~ShadowMap() {
        Cleanup();
    }

    void ShadowMap::Cleanup() {
        if (m_FBO != 0) {
            glDeleteFramebuffers(1, &m_FBO);
            m_FBO = 0;
        }
        if (m_DepthTexture != 0) {
            glDeleteTextures(1, &m_DepthTexture);
            m_DepthTexture = 0;
        }
    }

    void ShadowMap::Invalidate() {
        Cleanup();

        // 1. Create Framebuffer Object using DSA
        glCreateFramebuffers(1, &m_FBO);

        // 2. Allocate and configure Depth Texture using DSA
        glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthTexture);
        
        // Immutable storage allocation for high-precision depth component
        glTextureStorage2D(m_DepthTexture, 1, GL_DEPTH_COMPONENT32F, m_Width, m_Height);

        // Configure texture filters and wrapping
        glTextureParameteri(m_DepthTexture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_DepthTexture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_DepthTexture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTextureParameteri(m_DepthTexture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // Configure hardware shadow comparison mode for sampler2DShadow
        glTextureParameteri(m_DepthTexture, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTextureParameteri(m_DepthTexture, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        // Set border color to white (1.0) so that coordinates outside the shadow map are treated as unshadowed
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTextureParameterfv(m_DepthTexture, GL_TEXTURE_BORDER_COLOR, borderColor);

        // 3. Attach texture to FBO
        glNamedFramebufferTexture(m_FBO, GL_DEPTH_ATTACHMENT, m_DepthTexture, 0);

        // 4. Set Draw/Read buffers to GL_NONE as this is a depth-only Framebuffer
        glNamedFramebufferDrawBuffer(m_FBO, GL_NONE);
        glNamedFramebufferReadBuffer(m_FBO, GL_NONE);

        // 5. Verify status
        if (glCheckNamedFramebufferStatus(m_FBO, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("ShadowMap Framebuffer creation failed: FBO is incomplete!");
        }
    }

    void ShadowMap::Bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    }

    void ShadowMap::Unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

} // namespace Avalon
