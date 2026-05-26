#pragma once

#include <vector>
#include <memory>

namespace Avalon {

    enum class FramebufferTextureFormat {
        None = 0,
        // Color formats
        RGBA8,
        RGBA16F,
        // Depth/Stencil formats
        DEPTH24STENCIL8,
        // Depth only component (critical for shadow maps)
        DEPTH_COMPONENT
    };

    struct FramebufferTextureSpecification {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format)
            : TextureFormat(format) {}

        FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
    };

    struct FramebufferAttachmentSpecification {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
            : Attachments(attachments) {}

        std::vector<FramebufferTextureSpecification> Attachments;
    };

    struct FramebufferSpecification {
        uint32_t Width = 0;
        uint32_t Height = 0;
        FramebufferAttachmentSpecification Attachments;
        uint32_t Samples = 1;
        bool SwapChainTarget = false;
    };

    class Framebuffer {
    public:
        explicit Framebuffer(const FramebufferSpecification& spec);
        ~Framebuffer();

        Framebuffer(const Framebuffer&) = delete;
        Framebuffer& operator=(const Framebuffer&) = delete;

        void Bind();
        void Unbind();
        void Resize(uint32_t width, uint32_t height);

        uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const { return m_ColorAttachments[index]; }
        uint32_t GetDepthAttachmentRendererID() const { return m_DepthAttachment; }

        const FramebufferSpecification& GetSpecification() const { return m_Specification; }

    private:
        void Invalidate();
        void Cleanup();

    private:
        uint32_t m_RendererID = 0;
        FramebufferSpecification m_Specification;

        std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
        FramebufferTextureSpecification m_DepthAttachmentSpec = FramebufferTextureFormat::None;

        std::vector<uint32_t> m_ColorAttachments;
        uint32_t m_DepthAttachment = 0;
    };

} // namespace Avalon
