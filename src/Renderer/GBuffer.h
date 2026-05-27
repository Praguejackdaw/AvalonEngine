#pragma once

#include <stdint.h>
#include <memory>

namespace Avalon {

    class GBuffer {
    public:
        static std::unique_ptr<GBuffer> Create(uint32_t width, uint32_t height);
        
        GBuffer(uint32_t width, uint32_t height);
        ~GBuffer();

        GBuffer(const GBuffer&) = delete;
        GBuffer& operator=(const GBuffer&) = delete;

        void Bind();
        void Unbind();
        void Resize(uint32_t width, uint32_t height);

        // Getters for individual attachment textures
        uint32_t GetNormalTexture() const { return m_NormalTex; }
        uint32_t GetAlbedoTexture() const { return m_AlbedoTex; }
        uint32_t GetMetallicRoughnessTexture() const { return m_MetallicRoughnessTex; }
        uint32_t GetDepthTexture() const { return m_DepthTex; }

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }

    private:
        void Invalidate();
        void Cleanup();

    private:
        uint32_t m_FBO = 0;
        uint32_t m_NormalTex = 0;               // RGB16F
        uint32_t m_AlbedoTex = 0;               // RGBA8
        uint32_t m_MetallicRoughnessTex = 0;    // RG16F
        uint32_t m_DepthTex = 0;                // DEPTH32F

        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };

} // namespace Avalon
