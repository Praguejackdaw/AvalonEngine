#pragma once

#include "Renderer/RenderCommand.h"
#include <glm/glm.hpp>

namespace Avalon {

    class VertexArray;
    class Shader;
    class Camera;

    class Renderer {
    public:
        static void Init();
        static void Shutdown();

        static void Update(float deltaTime);
        static void Clear();

        static void BeginScene(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        static void EndScene();

        static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

    private:
        struct SceneData {
            glm::mat4 ViewMatrix;
            glm::mat4 ProjectionMatrix;
        };

        static std::unique_ptr<SceneData> s_SceneData;
    };

} // namespace Avalon
