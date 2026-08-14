cmake_minimum_required(VERSION 3.16)
project(3DRenderer VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

add_executable(
        renderer src/main.cpp
        src/math/Mat4.cpp
        src/math/Vect3.cpp
        src/rendering/Mesh.cpp
        src/rendering/Renderer.cpp)

target_link_libraries(renderer PRIVATE SDL2::SDL2 SDL2::SDL2main)