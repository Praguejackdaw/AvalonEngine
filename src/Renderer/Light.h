#pragma once

#include <glm/glm.hpp>

namespace Avalon {

    struct DirectionalLight {
        glm::vec3 Direction = { -0.2f, -1.0f, -0.3f };
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        float Intensity = 1.0f;
    };

    struct PointLight {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        float Intensity = 1.0f;

        float Constant = 1.0f;
        float Linear = 0.09f;
        float Quadratic = 0.032f;
    };

} // namespace Avalon
