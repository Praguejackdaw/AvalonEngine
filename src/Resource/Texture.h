#pragma once

#include <string>

namespace Avalon {

    class Texture {
    public:
        explicit Texture(const std::string& path);
        ~Texture();

        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        void Bind(uint32_t slot = 0) const;

        uint32_t GetWidth() const { return m_Width; }
        uint32_t GetHeight() const { return m_Height; }
        uint32_t GetRendererID() const { return m_RendererID; }

        const std::string& GetPath() const { return m_Path; }

    private:
        uint32_t m_RendererID = 0;
        std::string m_Path;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
    };

} // namespace Avalon
