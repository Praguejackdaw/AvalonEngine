#pragma once

#include <glm/glm.hpp>

namespace Avalon {

    struct alignas(16) DirectionalLight {
        glm::vec3 Direction = { -0.2f, -1.0f, -0.3f };
        float Padding1 = 0.0f; // Align to 16 bytes
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        float Intensity = 1.0f; // Align to 16 bytes
    };

    struct alignas(16) PointLight {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        float Padding1 = 0.0f; // Align to 16 bytes
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        float Intensity = 1.0f; // Align to 16 bytes

        float Constant = 1.0f;
        float Linear = 0.09f;
        float Quadratic = 0.032f;
        float Padding2 = 0.0f; // Align to 16 bytes
    };

} // namespace Avalon
