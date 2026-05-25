#include "Core/Application.h"
#include <imgui.h>
#include "Core/Window.h"
#include "UI/UIManager.h"
#include "Renderer/Renderer.h"
#include "Renderer/RenderCommand.h"
#include "Camera/FPSCamera.h"
#include "Shader/Shader.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Buffer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Avalon {

    Application* Application::s_Instance = nullptr;

    Application::Application(const ApplicationSpecification& spec)
        : m_Specification(spec) {
        if (s_Instance) {
            throw std::runtime_error("Application already exists!");
        }
        s_Instance = this;

        Init();
    }

    Application::~Application() {
        Shutdown();
        s_Instance = nullptr;
    }

    void Application::Init() {
        // Initialize Window
        m_Window = std::make_unique<Window>(WindowProps(m_Specification.Name, m_Specification.Width, m_Specification.Height));
        m_Window->SetVSync(m_Specification.VSync);

        // Initialize Renderer base configurations
        Renderer::Init();

        // Initialize UI Layer (ImGui)
        m_UIManager = std::make_unique<UIManager>();
        m_UIManager->Init();

        // Setup Euler-based FPS roaming camera
        m_Camera = std::make_unique<FPSCamera>(
            45.0f, 
            static_cast<float>(m_Window->GetWidth()) / static_cast<float>(m_Window->GetHeight()), 
            0.1f, 
            100.0f
        );
        m_Camera->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));

        // Load Default Shader (will read vertex/fragment code)
        m_Shader = std::make_shared<Shader>("assets/shaders/default.glsl");

        // Define faceted procedural 3D Cube (36 vertices with isolated normal vectors)
        float cubeVertices[] = {
            // Position          // Normals          // TexCoords
            // Back face (Normal: 0, 0, -1) -> CCW: BBR -> BBL -> BTL, BTL -> BTR -> BBR
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // BBR
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f, // BBL
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, // BTL
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f, // BTL
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f, // BTR
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, // BBR

            // Front face (Normal: 0, 0, 1) -> CCW: FBL -> FBR -> FTR, FTR -> FTL -> FBL
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // FBL
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f, // FBR
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // FTR
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f, // FTR
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f, // FTL
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f, // FBL

            // Left face (Normal: -1, 0, 0) -> CCW: BBL -> FBL -> FTL, FTL -> BTL -> BBL
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // BBL
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // FBL
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // FTL
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // FTL
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // BTL
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // BBL

            // Right face (Normal: 1, 0, 0) -> CCW: FBR -> BBR -> BTR, BTR -> FTR -> FBR
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // FBR
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f, // BBR
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // BTR
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, // BTR
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, // FTR
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, // FBR

            // Bottom face (Normal: 0, -1, 0) -> CCW: BBL -> BBR -> FBR, FBR -> FBL -> BBL
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // BBL
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, // BBR
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // FBR
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f, // FBR
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f, // FBL
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f, // BBL

            // Top face (Normal: 0, 1, 0) -> CCW: FTL -> FTR -> BTR, BTR -> BTL -> FTL
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f, // FTL
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f, // FTR
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, // BTR
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f, // BTR
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f, // BTL
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f  // FTL
        };

        uint32_t cubeIndices[36];
        for (uint32_t i = 0; i < 36; i++) {
            cubeIndices[i] = i;
        }

        m_CubeVAO = std::make_shared<VertexArray>();

        // Construct VBO
        auto vertexBuffer = std::make_shared<VertexBuffer>(cubeVertices, sizeof(cubeVertices), false);
        vertexBuffer->SetLayout({
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float3, "a_Normal"   },
            { ShaderDataType::Float2, "a_TexCoords" }
        });
        m_CubeVAO->AddVertexBuffer(vertexBuffer);

        // Construct EBO
        auto indexBuffer = std::make_shared<IndexBuffer>(cubeIndices, 36);
        m_CubeVAO->SetIndexBuffer(indexBuffer);
    }

    void Application::Shutdown() {
        m_UIManager->Shutdown();
        Renderer::Shutdown();
        std::cout << "Engine successfully shut down." << std::endl;
    }

    void Application::Run() {
        while (m_Running) {
            float time = static_cast<float>(glfwGetTime());
            float deltaTime = time - m_LastFrameTime;
            m_LastFrameTime = time;

            // 1. Process OS Events / Input
            m_Window->OnUpdate();

            if (glfwWindowShouldClose(m_Window->GetNativeWindow())) {
                m_Running = false;
            }

            // 2. Logic Update
            OnUpdate(deltaTime);

            // 3. Graphics Rendering
            OnRender();
        }
    }

    void Application::Close() {
        m_Running = false;
    }

    void Application::OnUpdate(float deltaTime) {
        Renderer::Update(deltaTime);

        // Resize camera aspect ratio if viewport size changed
        m_Camera->SetViewportSize(m_Window->GetWidth(), m_Window->GetHeight());

        // Update camera position/rotation via keyboard and mouse polling
        m_Camera->OnUpdate(deltaTime);

        // Spin the cube dynamically based on auto-rotation settings
        if (m_AutoRotate) {
            m_RotationAngle += m_RotationSpeed * deltaTime;
            if (m_RotationAngle > 360.0f) {
                m_RotationAngle -= 360.0f;
            }
        }
    }

    void Application::OnRender() {
        // Set clear color dynamically
        RenderCommand::SetClearColor(m_ClearColor);
        Renderer::Clear();

        // Bind Shader and calculate Model Transform
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(m_RotationAngle), glm::vec3(0.5f, 1.0f, 0.0f));

        // Submit geometry to the Renderer pipeline
        Renderer::BeginScene(m_Camera->GetViewMatrix(), m_Camera->GetProjectionMatrix());
        
        // Render cube using Blinn-Phong diffuse lighting shader
        m_Shader->Bind();
        m_Shader->SetFloat3("u_ViewPos", m_Camera->GetPosition());
        Renderer::Submit(m_Shader, m_CubeVAO, model);

        Renderer::EndScene();

        // Render Developer UI overlays (ImGui)
        m_UIManager->Begin();
        
        // Compact, gorgeous, unified controls and diagnostics overlay
        ImGui::Begin("Avalon Engine Panel v0.1", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        
        if (ImGui::CollapsingHeader("System Diagnostics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("FPS: %.1f FPS", ImGui::GetIO().Framerate);
            ImGui::Text("Frame Time: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
            ImGui::Text("Graphics API: OpenGL 4.5 Core Profile (DSA)");
            
            const char* gpuName = (const char*)glGetString(GL_RENDERER);
            ImGui::Text("GPU: %s", gpuName ? gpuName : "Unknown");
        }

        if (ImGui::CollapsingHeader("Camera Roaming Debugger", ImGuiTreeNodeFlags_DefaultOpen)) {
            glm::vec3 pos = m_Camera->GetPosition();
            ImGui::Text("Position: [X:%.2f, Y:%.2f, Z:%.2f]", pos.x, pos.y, pos.z);
            ImGui::Text("Orientation: [Yaw:%.1f, Pitch:%.1f]", m_Camera->GetYaw(), m_Camera->GetPitch());
            ImGui::Separator();
            ImGui::TextDisabled("Controls: Hold RMB + WASD/QE");
        }

        if (ImGui::CollapsingHeader("Simulation Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Auto-Rotate Object", &m_AutoRotate);
            if (m_AutoRotate) {
                ImGui::SliderFloat("Rotation Speed (deg/s)", &m_RotationSpeed, 0.0f, 180.0f);
            } else {
                ImGui::SliderFloat("Manual Angle", &m_RotationAngle, 0.0f, 360.0f);
            }
            
            ImGui::Separator();
            
            // Dynamic clear color edit
            ImGui::ColorEdit3("Viewport Background", &m_ClearColor.r);
        }
        
        ImGui::End();

        m_UIManager->End();
    }

} // namespace Avalon
