#pragma once

#include <stdint.h>
#include <memory>

namespace Avalon {

    enum class ShadowType {
        Directional = 0,
        Cascaded = 1,     // Future extension (GL_TEXTURE_2D_ARRAY)
        PointLight = 2    // Future extension (GL_TEXTURE_CUBE_MAP)
    };

    /**
     * @brief High-performance Depth-Only FBO wrapper using OpenGL 4.5 DSA.
     */
    class ShadowMap {
    public:
        static std::unique_ptr<ShadowMap> Create(uint32_t width, uint32_t height, ShadowType type = ShadowType::Directional);
        
        ShadowMap(uint32_t width, uint32_t height, ShadowType type);
        ~ShadowMap();

        ShadowMap(const ShadowMap&) = delete;
        ShadowMap& operator=(const ShadowMap&) = delete;

        void Bind();
        void Unbind();

        uint32_t GetDepthTextureID() const { return m_DepthTexture; }
        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        ShadowType GetType() const { return m_Type; }

    private:
        void Invalidate();
        void Cleanup();

    private:
        uint32_t m_FBO = 0;
        uint32_t m_DepthTexture = 0;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        ShadowType m_Type;
    };

} // namespace Avalon
