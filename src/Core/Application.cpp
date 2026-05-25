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
#include "Resource/Scene.h"
#include "Renderer/Framebuffer.h"
#include "Resource/ResourceManager.h"
#include "Renderer/ForwardRenderer.h"
#include "Resource/Material.h"

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

        // Initialize ForwardRenderer Orchestrator
        m_Renderer = ForwardRenderer::Create();
        m_Renderer->Init();

        // Initialize UI Layer (ImGui)
        m_UIManager = std::make_unique<UIManager>();
        m_UIManager->Init();

        // Setup Euler-based FPS roaming camera
        m_Camera = std::make_shared<FPSCamera>(
            45.0f, 
            static_cast<float>(m_Window->GetWidth()) / static_cast<float>(m_Window->GetHeight()), 
            0.1f, 
            100.0f
        );
        m_Camera->SetPosition(glm::vec3(0.0f, 0.0f, 5.0f));

        // Initialize scene and framebuffer allocations
        m_Scene = std::make_unique<Scene>();

        FramebufferSpecification fbSpec;
        fbSpec.Width = m_Window->GetWidth();
        fbSpec.Height = m_Window->GetHeight();
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH24STENCIL8 };
        m_Framebuffer = std::make_unique<Framebuffer>(fbSpec);

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
        m_Scene.reset();
        m_Framebuffer.reset();
        if (m_Renderer) {
            m_Renderer->Shutdown();
            m_Renderer.reset();
        }
        ResourceManager::Clear();

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

        // Dynamic Framebuffer resize to match window dimension
        if (m_Window->GetWidth() != m_Framebuffer->GetSpecification().Width ||
            m_Window->GetHeight() != m_Framebuffer->GetSpecification().Height) {
            m_Framebuffer->Resize(m_Window->GetWidth(), m_Window->GetHeight());
        }

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
        // 1. Off-screen Rendering Pass (Render Scene to custom Framebuffer)
        m_Framebuffer->Bind();
        
        RenderCommandAPI::SetClearColor(m_ClearColor);
        Renderer::Clear();

        // Bind Shader and calculate Model Transform
        glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(m_RotationAngle), glm::vec3(0.5f, 1.0f, 0.0f));

        // Begin ForwardRenderer Frame Setup
        m_Renderer->BeginFrame(m_Camera);

        // Retrieve and configure standard material
        static std::shared_ptr<Material> defaultMaterial = nullptr;
        if (!defaultMaterial) {
            defaultMaterial = BlinnPhongMaterial::Create();
        }

        // Submit draw package
        m_Renderer->Submit(m_CubeVAO, m_Shader, defaultMaterial, model);

        // Update active LightManager UBO structures
        auto& lightManager = m_Renderer->GetLightManager();
        if (lightManager) {
            lightManager->ClearLights();

            // Directional Light
            DirectionalLight dirLight;
            dirLight.Direction = m_DirLightDir;
            dirLight.Color = m_DirLightColor;
            dirLight.Intensity = m_DirLightIntensity;
            lightManager->SetDirectionalLight(dirLight);

            // Point Light
            PointLight pLight;
            pLight.Position = m_PointLightPos;
            pLight.Color = m_PointLightColor;
            pLight.Intensity = m_PointLightIntensity;
            pLight.Constant = 1.0f;
            pLight.Linear = 0.09f;
            pLight.Quadratic = 0.032f;
            lightManager->AddPointLight(pLight);

            // Spot Light (places at camera position looking forward)
            SpotLight sLight;
            sLight.Position = m_Camera->GetPosition();
            
            // Derive camera forward vector from view matrix (third column represents camera z-axis, we invert it for forward)
            glm::mat4 view = m_Camera->GetViewMatrix();
            glm::vec3 cameraForward = -glm::vec3(view[0][2], view[1][2], view[2][2]);
            
            sLight.Direction = cameraForward;
            sLight.Color = m_SpotLightColor;
            sLight.Intensity = m_SpotLightIntensity;
            sLight.CutOff = glm::cos(glm::radians(m_SpotLightAngle));
            sLight.OuterCutOff = glm::cos(glm::radians(m_SpotLightOuterAngle));
            sLight.Constant = 1.0f;
            sLight.Linear = 0.09f;
            sLight.Quadratic = 0.032f;
            lightManager->AddSpotLight(sLight);
        }

        // Execute drawing pipeline phases
        m_Renderer->EndFrame();
        
        m_Framebuffer->Unbind();

        // 2. On-screen Pass (Clear main screen and draw GUI)
        RenderCommandAPI::SetClearColor(glm::vec4(0.05f, 0.05f, 0.06f, 1.0f));
        Renderer::Clear();

        // Render Developer UI overlays (ImGui)
        m_UIManager->Begin();
        
        // Compact, gorgeous, unified controls and diagnostics overlay
        ImGui::Begin("Avalon Engine Panel v0.3", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        
        if (ImGui::CollapsingHeader("System Diagnostics", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("FPS: %.1f FPS", ImGui::GetIO().Framerate);
            ImGui::Text("Frame Time: %.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
            ImGui::Text("Active Draw Calls: %u", m_Renderer ? m_Renderer->GetDrawCallCount() : 0);
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

        if (ImGui::CollapsingHeader("Global Lighting Adjuster", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Directional Light:");
            ImGui::SliderFloat3("Direction##Dir", &m_DirLightDir.x, -1.0f, 1.0f);
            m_DirLightDir = glm::normalize(m_DirLightDir);
            ImGui::ColorEdit3("Color##Dir", &m_DirLightColor.x);
            ImGui::SliderFloat("Intensity##Dir", &m_DirLightIntensity, 0.0f, 5.0f);

            ImGui::Separator();
            ImGui::Text("Point Light:");
            ImGui::SliderFloat3("Position##Point", &m_PointLightPos.x, -10.0f, 10.0f);
            ImGui::ColorEdit3("Color##Point", &m_PointLightColor.x);
            ImGui::SliderFloat("Intensity##Point", &m_PointLightIntensity, 0.0f, 10.0f);

            ImGui::Separator();
            ImGui::Text("Flashlight (Spot Light):");
            ImGui::ColorEdit3("Color##Spot", &m_SpotLightColor.x);
            ImGui::SliderFloat("Intensity##Spot", &m_SpotLightIntensity, 0.0f, 20.0f);
            ImGui::SliderFloat("Inner Angle##Spot", &m_SpotLightAngle, 5.0f, 45.0f);
            ImGui::SliderFloat("Outer Angle##Spot", &m_SpotLightOuterAngle, 10.0f, 60.0f);
            
            // Ensure inner cutoff doesn't exceed outer cutoff
            if (m_SpotLightAngle > m_SpotLightOuterAngle) {
                m_SpotLightOuterAngle = m_SpotLightAngle + 2.0f;
            }
        }

        if (ImGui::CollapsingHeader("Simulation Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Checkbox("Auto-Rotate Object", &m_AutoRotate);
            if (m_AutoRotate) {
                ImGui::SliderFloat("Rotation Speed (deg/s)", &m_RotationSpeed, 0.0f, 180.0f);
            } else {
                ImGui::SliderFloat("Manual Angle", &m_RotationAngle, 0.0f, 360.0f);
            }
            
            ImGui::Separator();
            
            bool wireframe = m_Renderer ? m_Renderer->IsWireframeMode() : false;
            if (ImGui::Checkbox("Wireframe Mode", &wireframe)) {
                if (m_Renderer) {
                    m_Renderer->SetWireframeMode(wireframe);
                }
            }

            ImGui::Separator();
            
            // Dynamic clear color edit
            ImGui::ColorEdit3("Viewport Background", &m_ClearColor.r);
        }
        
        ImGui::End();

        // 3. Render the Viewport panel displaying the Framebuffer color texture!
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(1600.0f, 900.0f), ImGuiCond_FirstUseEver);
        ImGui::Begin("Render Viewport");
        
        uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        
        // Render texture flipped vertically because OpenGL textures are y-flipped
        ImGui::Image((void*)(intptr_t)textureID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
        
        ImGui::End();
        ImGui::PopStyleVar();

        m_UIManager->End();
    }

} // namespace Avalon
