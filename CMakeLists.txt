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

target_include_directories(gravitySim PRIVATE include)

target_link_libraries(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/libglfw3.a"
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
    "-framework OpenGL"
)
