#pragma once

#include <stdint.h>

namespace Avalon {

    /**
     * @brief Centralized state manager implementing lazy state evaluation
     *        and caching to prevent state leaks and redundant OpenGL API calls.
     */
    class RenderStateManager {
    public:
        static void Init();

        static void SetDepthTest(bool enabled);
        static void SetDepthFunc(uint32_t func);

        static void SetCullFace(bool enabled);
        static void SetCullFaceMode(uint32_t mode);
        static void SetFrontFace(uint32_t face);

        static void SetBlend(bool enabled);
        static void SetBlendFunc(uint32_t src, uint32_t dst);

        static void SetScissorTest(bool enabled);

        static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        /**
         * @brief Enforces expected 3D opaque pipeline rendering states.
         *        Guarantees that state changes from UI or other passes do not pollute opaque drawing.
         */
        static void ResetToDefault3DState();

    private:
        struct GLStateCache {
            bool DepthTest = false;
            uint32_t DepthFunc = 0;
            bool CullFace = false;
            uint32_t CullFaceMode = 0;
            uint32_t FrontFace = 0;
            bool Blend = false;
            uint32_t BlendSrc = 0;
            uint32_t BlendDst = 0;
            bool ScissorTest = false;
            uint32_t ViewportX = 0;
            uint32_t ViewportY = 0;
            uint32_t ViewportWidth = 0;
            uint32_t ViewportHeight = 0;
        };

        static GLStateCache s_Cache;
        static bool s_Initialized;
    };

} // namespace Avalon
