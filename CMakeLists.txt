cmake_minimum_required(VERSION 3.20)
project(GraphicsEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# ---- Engine library (everything except examples/) ----
file(GLOB_RECURSE ENGINE_SOURCES
    "${CMAKE_SOURCE_DIR}/src/core/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/math/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/rendering/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/Engine.cpp"
)

add_library(engine STATIC ${ENGINE_SOURCES})

target_include_directories(engine PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/third_party
)

target_link_libraries(engine PUBLIC
    Vulkan::Vulkan
    glfw
)

target_compile_options(engine PRIVATE -Wall -Wextra)

# ---- Shaders ----
find_program(GLSLC glslc REQUIRED)

file(GLOB SHADER_SOURCES
    "${CMAKE_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_SOURCE_DIR}/shaders/*.frag"
)

set(SPIRV_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY ${SPIRV_OUTPUT_DIR})

set(SPIRV_BINARIES "")
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPIRV_OUT "${SPIRV_OUTPUT_DIR}/${SHADER_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV_OUT}
        COMMAND ${GLSLC} ${SHADER} -o ${SPIRV_OUT}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader: ${SHADER_NAME}"
    )
    list(APPEND SPIRV_BINARIES ${SPIRV_OUT})
endforeach()

add_custom_target(Shaders DEPENDS ${SPIRV_BINARIES})

# ---- Examples: one executable per file in src/examples/ ----
function(add_example target_name source)
    add_executable(${target_name} ${source})
    target_link_libraries(${target_name} PRIVATE engine)
    target_compile_options(${target_name} PRIVATE -Wall -Wextra)
    add_dependencies(${target_name} Shaders)
    # Stage assets next to the built shaders so the binary can find them
    # when run from the build directory.
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
    )
endfunction()

add_example(Cubes "${CMAKE_SOURCE_DIR}/src/examples/Cubes.cpp")
add_example(TextureCubes "${CMAKE_SOURCE_DIR}/src/examples/TextureCubes.cpp")
add_example(Sprites "${CMAKE_SOURCE_DIR}/src/examples/Sprites.cpp")
add_example(Text "${CMAKE_SOURCE_DIR}/src/examples/Text.cpp")
add_example(Hud "${CMAKE_SOURCE_DIR}/src/examples/Hud.cpp")
