#pragma once

#include "Renderer/Light.h"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace Avalon {

    /**
     * @brief SpotLight structure defined to match std140 layouts perfectly.
     */
    struct alignas(16) SpotLight {
        glm::vec3 Position = { 0.0f, 0.0f, 0.0f };
        float CutOff = 0.976f;              // cos of 12.5 degrees
        glm::vec3 Direction = { 0.0f, -1.0f, 0.0f };
        float OuterCutOff = 0.953f;         // cos of 17.5 degrees
        
        glm::vec3 Color = { 1.0f, 1.0f, 1.0f };
        float Intensity = 1.0f;
        
        float Constant = 1.0f;
        float Linear = 0.09f;
        float Quadratic = 0.032f;
        float Padding = 0.0f;               // Pad structure to 64 bytes
    };

    /**
     * @brief Interface and factory for managing multi-light environments and GPU buffer packing.
     */
    class LightManager {
    public:
        virtual ~LightManager() = default;

        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        virtual void SetDirectionalLight(const DirectionalLight& light) = 0;
        virtual void AddPointLight(const PointLight& light) = 0;
        virtual void AddSpotLight(const SpotLight& light) = 0;
        virtual void ClearLights() = 0;

        virtual const DirectionalLight& GetDirectionalLight() const = 0;
        virtual const std::vector<PointLight>& GetPointLights() const = 0;
        virtual const std::vector<SpotLight>& GetSpotLights() const = 0;

        // Packs and updates the Light UBO using glNamedBufferSubData
        virtual void UpdateGPUBuffer() = 0;

        // Binds the active light UBO to a specific binding point
        virtual void BindLightBuffer(uint32_t bindingPoint) = 0;

        static std::shared_ptr<LightManager> Create();
    };

} // namespace Avalon
