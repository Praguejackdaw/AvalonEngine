cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(AvalonEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure output directories for built binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include(FetchContent)

# 1. GLFW Configuration & Fetching
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

message(STATUS "Fetching GLFW...")
FetchContent_Declare(
    glfw
    URL https://github.com/glfw/glfw/archive/refs/tags/3.4.zip
)
FetchContent_MakeAvailable(glfw)

# 2. GLAD Configuration & Fetching (OpenGL 4.5 Core Profile)
message(STATUS "Fetching GLAD...")
FetchContent_Declare(
    glad
    URL https://github.com/Dav1dde/glad/archive/refs/tags/v0.1.36.zip
)
# GLAD options for code generation
set(GLAD_PROFILE "core" CACHE STRING "" FORCE)
set(GLAD_API "gl=4.5" CACHE STRING "" FORCE)
set(GLAD_EXPORT ON CACHE BOOL "" FORCE)
set(GLAD_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glad)

# 3. GLM (Header Only math library)
message(STATUS "Fetching GLM...")
FetchContent_Declare(
    glm
    URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip
)
FetchContent_MakeAvailable(glm)

# 4. Assimp (Highly optimized for fast compilation)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
# Enable only specific lightweight & standard importers
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_INFO OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)

message(STATUS "Fetching Assimp...")
FetchContent_Declare(
    assimp
    URL https://github.com/assimp/assimp/archive/refs/tags/v5.4.1.zip
)
FetchContent_MakeAvailable(assimp)

# 5. Dear ImGui configuration
message(STATUS "Fetching ImGui...")
FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.90.6-docking.zip
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    
    # Custom ImGui library setup including GLFW and OpenGL3 backends
    add_library(imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
    target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
    )
    target_link_libraries(imgui PUBLIC glfw glad)
endif()

# Recurse into src directory for core engine and application executable
add_subdirectory(src)
