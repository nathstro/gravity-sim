cmake_minimum_required(VERSION 3.20)
project(gravitySim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(gravitySim
    src/main.cpp
    src/Renderer.cpp
    src/Body.cpp
    src/Camera.cpp
    src/Shader.cpp
    src/glad.c
    src/models/Sphere.cpp
)

add_library(imgui
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
)

target_include_directories(gravitySim PRIVATE include)
target_include_directories(imgui PUBLIC imgui include)

target_link_libraries(gravitySim PRIVATE
    imgui
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libglfw3.a"
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
    "-framework OpenGL"
)
