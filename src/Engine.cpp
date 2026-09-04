#include "Engine.hpp"
#include <GLFW/glfw3.h>

Engine::Engine(int width, int height, const std::string& title) {
    window = std::make_unique<Window>(width, height, title);
    vulkanContext = std::make_unique<VulkanContext>();
    vulkanContext->init(*window);
    renderer = std::make_unique<Renderer>();
    camera = std::make_unique<Camera>();
    input = std::make_unique<Input>(*window);
}

Engine::~Engine() {
    if (vulkanContext) {
        vulkanContext->waitIdle();
        vulkanContext->cleanup();
    }
}

void Engine::run(const std::function<void(float deltaTime, Renderer& renderer, Camera& camera, Input& input)>& updateCallback) {
    float lastTime = static_cast<float>(glfwGetTime());

    while (!window->shouldClose()) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        window->pollEvents();
        input->update();

        int width = window->getWidth();
        int height = window->getHeight();
        if (width == 0 || height == 0) {
            continue; // minimized — nothing to render, avoid divide-by-zero aspect ratio
        }

        renderer->beginFrame();

        if (updateCallback) {
            updateCallback(deltaTime, *renderer, *camera, *input);
        }

        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        Mat4 viewProjection = camera->getProjectionMatrix(aspectRatio) * camera->getViewMatrix();
        vulkanContext->setViewProjection(viewProjection);

        vulkanContext->updateVertexBuffer(renderer->getVertices());
        vulkanContext->drawFrame(*window);
    }
}
