#include "UI/UIManager.h"
#include "Core/Application.h"
#include "Core/Window.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace Avalon {

    UIManager::UIManager() {}

    UIManager::~UIManager() {}

    void UIManager::Init() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Disable ImGui keyboard navigation to free W,S,A,D for camera roaming
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Disable Multi-Viewport to keep OS focus on the main GLFW window for stable WASD inputs

        // Setup Dear ImGui style (Modern Dark & Sleek)
        ImGui::StyleColorsDark();

        // When viewports are enabled, we adjust WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Apply fine-tuned sleek theme configurations
        style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
        style.Colors[ImGuiCol_Header]        = ImVec4(0.20f, 0.20f, 0.22f, 1.0f);
        style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.32f, 1.0f);
        style.Colors[ImGuiCol_HeaderActive]  = ImVec4(0.15f, 0.15f, 0.17f, 1.0f);
        style.Colors[ImGuiCol_Button]        = ImVec4(0.20f, 0.40f, 0.70f, 1.0f);
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.48f, 0.80f, 1.0f);
        style.Colors[ImGuiCol_ButtonActive]  = ImVec4(0.15f, 0.32f, 0.60f, 1.0f);

        auto* window = Application::Get().GetWindow().GetNativeWindow();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 450");
    }

    void UIManager::Shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void UIManager::Begin() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void UIManager::End() {
        ImGuiIO& io = ImGui::GetIO();
        Application& app = Application::Get();
        io.DisplaySize = ImVec2(static_cast<float>(app.GetWindow().GetWidth()), static_cast<float>(app.GetWindow().GetHeight()));

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows (when Multi-Viewports are enabled)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }
} // namespace Avalon
