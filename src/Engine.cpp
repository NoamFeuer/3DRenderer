#include "Engine.hpp"
#include <GLFW/glfw3.h>

Engine::Engine(int width, int height, const std::string& title) {
    window = std::make_unique<Window>(width, height, title);
    vulkanContext = std::make_unique<VulkanContext>();
    vulkanContext->init(*window);
    renderer = std::make_unique<Renderer>();
}

Engine::~Engine() {
    if (vulkanContext) {
        vulkanContext->waitIdle();
        vulkanContext->cleanup();
    }
}

void Engine::run(const std::function<void(float deltaTime, Renderer& renderer)>& updateCallback) {
    float lastTime = static_cast<float>(glfwGetTime());

    while (!window->shouldClose()) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        window->pollEvents();

        renderer->beginFrame();

        if (updateCallback) {
            updateCallback(deltaTime, *renderer);
        }

        vulkanContext->updateVertexBuffer(renderer->getVertices());
        vulkanContext->drawFrame(*window);
    }
}
