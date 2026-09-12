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
        textures.clear();
        fonts.clear();
        vulkanContext->cleanup();
    }
}

int Engine::loadTexture(const std::string& path) {
    Texture texture = Texture::load(path, *vulkanContext);
    uint32_t index = vulkanContext->registerTexture(texture.getImageView());
    textures.push_back(std::move(texture));
    return static_cast<int>(index);
}

Font* Engine::loadFont(const std::string& path, int pixelHeight) {
    auto font = std::make_unique<Font>(Font::load(path, pixelHeight, *vulkanContext));
    Font* raw = font.get();
    fonts.push_back(std::move(font));
    return raw;
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
        if (width == 0 || height == 0)
            continue;

        renderer->beginFrame();

        if (updateCallback)
            updateCallback(deltaTime, *renderer, *camera, *input);

        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        Mat4 viewProjection = camera->getProjectionMatrix(aspectRatio) * camera->getViewMatrix();
        vulkanContext->setViewProjection(viewProjection);

        vulkanContext->updateVertexBuffer(renderer->getVertices());
        vulkanContext->updateIndexBuffer(renderer->getIndices());
        vulkanContext->updateModelMatrixBuffer(renderer->getModelMatrices());
        vulkanContext->setBlendedIndexOffset(renderer->getBlendedIndexOffset());
        vulkanContext->setScreenIndexOffset(renderer->getScreenIndexOffset());
        vulkanContext->drawFrame(*window);
    }
}

void Engine::runFixed(
    float fixedDeltaTime,
    const std::function<void(float deltaTime, Renderer& renderer, Camera& camera, Input& input)>& fixedUpdate,
    const std::function<void(float alpha, Renderer& renderer, Camera& camera, Input& input)>& renderCallback) {

    float lastTime = static_cast<float>(glfwGetTime());
    float accumulator = 0.0f;

    while (!window->shouldClose()) {
        float currentTime = static_cast<float>(glfwGetTime());
        float frameTime = currentTime - lastTime;
        lastTime = currentTime;

        // Clamp the per-frame delta so a hiccup can't trigger a huge catch-up
        // burst of fixed steps (the "spiral of death").
        if (frameTime > 0.25f)
            frameTime = 0.25f;

        window->pollEvents();
        input->update();

        int width = window->getWidth();
        int height = window->getHeight();
        if (width == 0 || height == 0)
            continue;

        accumulator += frameTime;
        while (accumulator >= fixedDeltaTime) {
            if (fixedUpdate)
                fixedUpdate(fixedDeltaTime, *renderer, *camera, *input);
            accumulator -= fixedDeltaTime;
        }

        float alpha = fixedDeltaTime > 0.0f ? accumulator / fixedDeltaTime : 0.0f;

        renderer->beginFrame();

        if (renderCallback)
            renderCallback(alpha, *renderer, *camera, *input);

        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        Mat4 viewProjection = camera->getProjectionMatrix(aspectRatio) * camera->getViewMatrix();
        vulkanContext->setViewProjection(viewProjection);

        vulkanContext->updateVertexBuffer(renderer->getVertices());
        vulkanContext->updateIndexBuffer(renderer->getIndices());
        vulkanContext->updateModelMatrixBuffer(renderer->getModelMatrices());
        vulkanContext->setBlendedIndexOffset(renderer->getBlendedIndexOffset());
        vulkanContext->setScreenIndexOffset(renderer->getScreenIndexOffset());
        vulkanContext->drawFrame(*window);
    }
}
