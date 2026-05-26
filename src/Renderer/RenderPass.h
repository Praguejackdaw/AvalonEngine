#pragma once

#include <memory>

namespace Avalon {

    class Scene;

    class RenderPass {
    public:
        virtual ~RenderPass() = default;

        virtual void Init() = 0;
        virtual void Execute(const std::shared_ptr<Scene>& scene) = 0;
    };

} // namespace Avalon
