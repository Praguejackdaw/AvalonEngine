#pragma once

namespace Avalon {

    class UIManager {
    public:
        UIManager();
        ~UIManager();

        void Init();
        void Shutdown();

        void Begin();
        void End();
    };

} // namespace Avalon
