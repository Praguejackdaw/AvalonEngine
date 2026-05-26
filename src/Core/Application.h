#pragma once

#include "Core/Timestep.h"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace Avalon {

    class Window;
    class UIManager;
    class Timestep;
    class FPSCamera;
    class Shader;
    class VertexArray;
    class Scene;
    class Framebuffer;
    class ForwardRenderer;

    struct ApplicationSpecification {
        std::string Name = "Avalon Engine";
        uint32_t Width = 1600;
        uint32_t Height = 900;
        bool VSync = true;
    };

    class Application {
    public:
        explicit Application(const ApplicationSpecification& spec = ApplicationSpecification());
        virtual ~Application();

        // Prevent copying to maintain strict ownership of the application instance
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        // Run the main game loop
        void Run();
        
        // Signal the application to terminate at the end of the current frame
        void Close();

        // Getters for subsystems
        Window& GetWindow() { return *m_Window; }
        UIManager& GetUIManager() { return *m_UIManager; }
        
        // Static singleton accessor for global system queries (e.g. Input, Window)
        static Application& Get() { return *s_Instance; }

    private:
        void Init();
        void Shutdown();
        
        // Engine frame lifecycle events
        void OnUpdate(float deltaTime);
        void OnRender();

    private:
        ApplicationSpecification m_Specification;
        std::unique_ptr<Window> m_Window;
        std::unique_ptr<UIManager> m_UIManager;
        
        std::shared_ptr<FPSCamera> m_Camera;
        std::shared_ptr<Shader> m_Shader;
        std::shared_ptr<VertexArray> m_CubeVAO;
        
        std::unique_ptr<Scene> m_Scene;
        std::unique_ptr<Framebuffer> m_Framebuffer;
        std::shared_ptr<ForwardRenderer> m_Renderer;

        bool m_Running = true;
        float m_LastFrameTime = 0.0f;
        float m_RotationAngle = 0.0f;
        
        // Control variables for dynamic settings
        bool m_AutoRotate = true;
        float m_RotationSpeed = 45.0f;
        glm::vec4 m_ClearColor = glm::vec4(0.08f, 0.08f, 0.10f, 1.00f);

        // Light adjustment parameters
        glm::vec3 m_DirLightDir = glm::vec3(-0.2f, -1.0f, -0.3f);
        glm::vec3 m_DirLightColor = glm::vec3(1.0f, 0.95f, 0.9f);
        float m_DirLightIntensity = 1.0f;

        glm::vec3 m_PointLightPos = glm::vec3(2.0f, 2.0f, 2.0f);
        glm::vec3 m_PointLightColor = glm::vec3(1.0f, 0.2f, 0.2f);
        float m_PointLightIntensity = 2.0f;

        glm::vec3 m_SpotLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        float m_SpotLightIntensity = 5.0f;
        float m_SpotLightAngle = 12.5f;
        float m_SpotLightOuterAngle = 17.5f;

        // PBR parameters
        glm::vec3 m_AlbedoFactor = glm::vec3(0.8f, 0.1f, 0.1f); // Beautiful custom primary color
        float m_MetallicFactor = 0.5f;
        float m_RoughnessFactor = 0.5f;
        float m_AOFactor = 1.0f;

        static Application* s_Instance;
    };

    // To be defined by the client / main.cpp (facilitates clean entry abstraction)
    Application* CreateApplication();

} // namespace Avalon
