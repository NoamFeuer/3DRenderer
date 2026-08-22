#include "Engine.hpp"
#include <GLFW/glfw3.h>

Engine::Engine(int width, int height, const std::string& title) {
    window = std::make_unique<Window>(width, height, title);
    vulkanContext = std::make_unique<VulkanContext>();
    vulkanContext->init(*window);
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

        if (updateCallback)
            updateCallback(deltaTime);

        vulkanContext->drawFrame(*window);
    }
}
