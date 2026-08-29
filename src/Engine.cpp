#include "Engine.hpp"
#include <GLFW/glfw3.h>

Engine::Engine(int width, int height, const std::string& title) {
    window = std::make_unique<Window>(width, height, title);
    vulkanContext = std::make_unique<VulkanContext>();

    // Temporary hardcoded triangle — this is where mesh loading will plug in
    // once Engine exposes a proper way to load/add geometry.
    std::vector<Vertex> triangleVertices = {
        { Vect3(0.0f, -0.5f, 0.0f), Vect3(1.0f, 0.0f, 0.0f) },
        { Vect3(0.5f,  0.5f, 0.0f), Vect3(0.0f, 1.0f, 0.0f) },
        { Vect3(-0.5f, 0.5f, 0.0f), Vect3(0.0f, 0.0f, 1.0f) },
    };

    vulkanContext->init(*window, triangleVertices);
}

Engine::~Engine() {
    if (vulkanContext) {
        vulkanContext->waitIdle();
        vulkanContext->cleanup();
    }
    // window and vulkanContext (unique_ptr) clean themselves up after this,
    // in reverse declaration order — vulkanContext first, then window.
}

void Engine::run(const std::function<void(float deltaTime)>& updateCallback) {
    float lastTime = static_cast<float>(glfwGetTime());

    while (!window->shouldClose()) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        window->pollEvents();

        if (updateCallback) {
            updateCallback(deltaTime);
        }

        vulkanContext->drawFrame(*window);
    }
}
