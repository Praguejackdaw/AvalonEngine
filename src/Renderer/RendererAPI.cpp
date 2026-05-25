#include "Renderer/RendererAPI.h"
#include "Renderer/VertexArray.h"
#include <glad/glad.h>
#include <iostream>

namespace Avalon {

    RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

    class OpenGLRendererAPI : public RendererAPI {
    public:
        void Init() override {
#ifdef _DEBUG
            // Enable high-fidelity GPU driver debugging callback
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
                switch (severity) {
                    case GL_DEBUG_SEVERITY_HIGH:
                        std::cerr << "[OpenGL High Error] " << message << std::endl;
                        break;
                    case GL_DEBUG_SEVERITY_MEDIUM:
                        std::cerr << "[OpenGL Medium Warning] " << message << std::endl;
                        break;
                    case GL_DEBUG_SEVERITY_LOW:
                        std::cout << "[OpenGL Low Info] " << message << std::endl;
                        break;
                    case GL_DEBUG_SEVERITY_NOTIFICATION:
                        // Ignore notifications to prevent log flooding
                        break;
                }
            }, nullptr);
#endif

            // Basic render state configurations
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glFrontFace(GL_CCW);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override {
            glViewport(x, y, width, height);
        }

        void SetClearColor(const glm::vec4& color) override {
            glClearColor(color.r, color.g, color.b, color.a);
        }

        void Clear() override {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray, uint32_t indexCount = 0) override {
            vertexArray->Bind();
            uint32_t count = indexCount == 0 ? vertexArray->GetIndexBuffer()->GetCount() : indexCount;
            glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
        }
    };

    std::unique_ptr<RendererAPI> RendererAPI::Create() {
        switch (s_API) {
            case RendererAPI::API::None:
                throw std::runtime_error("RendererAPI::None is not supported!");
            case RendererAPI::API::OpenGL:
                return std::make_unique<OpenGLRendererAPI>();
        }
        return nullptr;
    }

} // namespace Avalon
