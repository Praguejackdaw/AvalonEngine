#include "Core/Application.h"
#include <memory>

int main() {
    // Instantiate application with default specifications
    Avalon::ApplicationSpecification spec;
    spec.Name = "Avalon Engine - Realtime Renderer v0.1";
    spec.Width = 1600;
    spec.Height = 900;
    spec.VSync = true;

    auto app = std::make_unique<Avalon::Application>(spec);
    app->Run();
    
    return 0;
}
