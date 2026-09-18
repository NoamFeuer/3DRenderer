#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>

#include "core/Window.hpp"
#include "core/Input.hpp"
#include "rendering/VulkanContext.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Camera.hpp"
#include "rendering/Texture.hpp"
#include "rendering/Font.hpp"

class Engine {
public:
    Engine(int width, int height, const std::string& title);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void run(const std::function<void(float deltaTime, Renderer& renderer, Camera& camera, Input& input)>& updateCallback = nullptr);
    void runFixed(
        float fixedDeltaTime,
        const std::function<void(float deltaTime, Renderer& renderer, Camera& camera, Input& input)>& fixedUpdate,
        const std::function<void(float alpha, Renderer& renderer, Camera& camera, Input& input)>& renderCallback = nullptr);

    int getWindowWidth() const { return window ? window->getWidth() : 0; }
    int getWindowHeight() const { return window ? window->getHeight() : 0; }

    int loadTexture(const std::string& path);

    Font* loadFont(const std::string& path, int pixelHeight = 64);

    void setSkyColor(const Vect3& top, const Vect3& bottom);
    void setSkyEnabled(bool enabled);

private:
    std::unique_ptr<Window> window;
    std::unique_ptr<VulkanContext> vulkanContext;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Input> input;
    std::vector<Texture> textures;
    std::vector<std::unique_ptr<Font>> fonts;
};
