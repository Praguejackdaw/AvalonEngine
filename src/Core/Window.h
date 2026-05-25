#pragma once

#include <string>
#include <functional>

struct GLFWwindow;

namespace Avalon {

    struct WindowProps {
        std::string Title;
        uint32_t Width;
        uint32_t Height;

        WindowProps(const std::string& title = "Avalon Engine",
                    uint32_t width = 1600,
                    uint32_t height = 900)
            : Title(title), Width(width), Height(height) {}
    };

    // Interface representing a desktop system window
    class Window {
    public:
        using EventCallbackFn = std::function<void()>; // Simple layout for window close / resize callbacks if needed

        explicit Window(const WindowProps& props);
        ~Window();

        void OnUpdate();

        uint32_t GetWidth() const { return m_Data.Width; }
        uint32_t GetHeight() const { return m_Data.Height; }

        // Window attribute controls
        void SetVSync(bool enabled);
        bool IsVSync() const { return m_Data.VSync; }

        GLFWwindow* GetNativeWindow() const { return m_Window; }

    private:
        void Init(const WindowProps& props);
        void Shutdown();

    private:
        GLFWwindow* m_Window = nullptr;

        struct WindowData {
            std::string Title;
            uint32_t Width = 1600;
            uint32_t Height = 900;
            bool VSync = true;
        };

        WindowData m_Data;
    };

} // namespace Avalon
