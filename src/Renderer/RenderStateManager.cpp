#include "Renderer/RenderStateManager.h"
#include <glad/glad.h>

namespace Avalon {

    RenderStateManager::GLStateCache RenderStateManager::s_Cache = {};
    bool RenderStateManager::s_Initialized = false;

    void RenderStateManager::Init() {
        if (s_Initialized) return;

        s_Cache.DepthTest = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;
        glGetIntegerv(GL_DEPTH_FUNC, (GLint*)&s_Cache.DepthFunc);

        s_Cache.CullFace = glIsEnabled(GL_CULL_FACE) == GL_TRUE;
        glGetIntegerv(GL_CULL_FACE_MODE, (GLint*)&s_Cache.CullFaceMode);
        glGetIntegerv(GL_FRONT_FACE, (GLint*)&s_Cache.FrontFace);

        // Fetch initial blending states
        s_Cache.Blend = glIsEnabled(GL_BLEND) == GL_TRUE;
        glGetIntegerv(GL_BLEND_SRC, (GLint*)&s_Cache.BlendSrc);
        glGetIntegerv(GL_BLEND_DST, (GLint*)&s_Cache.BlendDst);

        s_Cache.ScissorTest = glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE;

        GLint vp[4];
        glGetIntegerv(GL_VIEWPORT, vp);
        s_Cache.ViewportX = static_cast<uint32_t>(vp[0]);
        s_Cache.ViewportY = static_cast<uint32_t>(vp[1]);
        s_Cache.ViewportWidth = static_cast<uint32_t>(vp[2]);
        s_Cache.ViewportHeight = static_cast<uint32_t>(vp[3]);

        s_Initialized = true;
    }

    void RenderStateManager::SetDepthTest(bool enabled) {
        Init();
        if (s_Cache.DepthTest != enabled) {
            if (enabled) {
                glEnable(GL_DEPTH_TEST);
            } else {
                glDisable(GL_DEPTH_TEST);
            }
            s_Cache.DepthTest = enabled;
        }
    }

    void RenderStateManager::SetDepthFunc(uint32_t func) {
        Init();
        if (s_Cache.DepthFunc != func) {
            glDepthFunc(func);
            s_Cache.DepthFunc = func;
        }
    }

    void RenderStateManager::SetCullFace(bool enabled) {
        Init();
        if (s_Cache.CullFace != enabled) {
            if (enabled) {
                glEnable(GL_CULL_FACE);
            } else {
                glDisable(GL_CULL_FACE);
            }
            s_Cache.CullFace = enabled;
        }
    }

    void RenderStateManager::SetCullFaceMode(uint32_t mode) {
        Init();
        if (s_Cache.CullFaceMode != mode) {
            glCullFace(mode);
            s_Cache.CullFaceMode = mode;
        }
    }

    void RenderStateManager::SetFrontFace(uint32_t face) {
        Init();
        if (s_Cache.FrontFace != face) {
            glFrontFace(face);
            s_Cache.FrontFace = face;
        }
    }

    void RenderStateManager::SetBlend(bool enabled) {
        Init();
        if (s_Cache.Blend != enabled) {
            if (enabled) {
                glEnable(GL_BLEND);
            } else {
                glDisable(GL_BLEND);
            }
            s_Cache.Blend = enabled;
        }
    }

    void RenderStateManager::SetBlendFunc(uint32_t src, uint32_t dst) {
        Init();
        if (s_Cache.BlendSrc != src || s_Cache.BlendDst != dst) {
            glBlendFunc(src, dst);
            s_Cache.BlendSrc = src;
            s_Cache.BlendDst = dst;
        }
    }

    void RenderStateManager::SetScissorTest(bool enabled) {
        Init();
        if (s_Cache.ScissorTest != enabled) {
            if (enabled) {
                glEnable(GL_SCISSOR_TEST);
            } else {
                glDisable(GL_SCISSOR_TEST);
            }
            s_Cache.ScissorTest = enabled;
        }
    }

    void RenderStateManager::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        Init();
        if (s_Cache.ViewportX != x || s_Cache.ViewportY != y || s_Cache.ViewportWidth != width || s_Cache.ViewportHeight != height) {
            glViewport(x, y, width, height);
            s_Cache.ViewportX = x;
            s_Cache.ViewportY = y;
            s_Cache.ViewportWidth = width;
            s_Cache.ViewportHeight = height;
        }
    }

    void RenderStateManager::ResetToDefault3DState() {
        SetDepthTest(true);
        SetDepthFunc(GL_LESS);
        SetCullFace(true);
        SetCullFaceMode(GL_BACK);
        SetFrontFace(GL_CCW);
        SetBlend(false);
        SetScissorTest(false);
    }

} // namespace Avalon
