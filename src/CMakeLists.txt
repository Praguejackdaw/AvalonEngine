add_executable(AvalonEngine
    main.cpp
    
    Core/Application.h
    Core/Application.cpp
    Core/Window.h
    Core/Window.cpp
    Core/Input.h
    Core/Input.cpp
    Core/Timestep.h
    Core/KeyCodes.h
    Core/MouseButtonCodes.h
    
    Renderer/Renderer.h
    Renderer/Renderer.cpp
    Renderer/RendererAPI.h
    Renderer/RendererAPI.cpp
    Renderer/RenderCommand.h
    Renderer/RenderCommand.cpp
    Renderer/VertexArray.h
    Renderer/VertexArray.cpp
    Renderer/Buffer.h
    Renderer/Buffer.cpp
    Renderer/Framebuffer.h
    Renderer/Framebuffer.cpp
    Renderer/Light.h
    Renderer/LightManager.h
    Renderer/LightManager.cpp
    Renderer/RenderQueue.h
    Renderer/RenderQueue.cpp
    Renderer/ForwardRenderer.h
    Renderer/ForwardRenderer.cpp
    Renderer/RenderStateManager.h
    Renderer/RenderStateManager.cpp
    Renderer/RenderPass.h
    Renderer/ShadowMap.h
    Renderer/ShadowMap.cpp
    Renderer/LightCamera.h
    Renderer/LightCamera.cpp
    Renderer/ShadowPass.h
    Renderer/ShadowPass.cpp
    
    Shader/Shader.h
    Shader/Shader.cpp
    
    Resource/Texture.h
    Resource/Texture.cpp
    Resource/Mesh.h
    Resource/Mesh.cpp
    Resource/Material.h
    Resource/Material.cpp
    Resource/Model.h
    Resource/Model.cpp
    Resource/ResourceManager.h
    Resource/ResourceManager.cpp
    Resource/Scene.h
    Resource/Scene.cpp
    
    Camera/Camera.h
    Camera/FPSCamera.h
    Camera/FPSCamera.cpp
    
    UI/UIManager.h
    UI/UIManager.cpp
)

target_include_directories(AvalonEngine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${glm_SOURCE_DIR}
)

target_link_libraries(AvalonEngine PRIVATE
    glfw
    glad
    assimp
    imgui
)

# Setup asset copying post-build
add_custom_command(TARGET AvalonEngine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:AvalonEngine>/assets
    COMMENT "Copying assets to runtime output directory..."
)
