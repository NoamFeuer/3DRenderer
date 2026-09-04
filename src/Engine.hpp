#pragma once

#include <string>
#include <functional>
#include <memory>

#include "core/Window.hpp"
#include "core/Input.hpp"
#include "rendering/VulkanContext.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Camera.hpp"

// Engine is the public entry point to the library.
// A user of this library should never need to see Window, VulkanContext,
// or any Vulkan/GLFW type directly — Renderer, Camera, and Input are the
// only surfaces they interact with.
class Engine {
public:
    Engine(int width, int height, const std::string& title);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    // Runs the main loop until the window is closed.
    // updateCallback is called once per frame — this is where a user of the
    // library writes their own logic, reads input, moves the camera, and
    // issues draw calls via `renderer`.
    void run(const std::function<void(float deltaTime, Renderer& renderer, Camera& camera, Input& input)>& updateCallback = nullptr);

private:
    std::unique_ptr<Window> window;
    std::unique_ptr<VulkanContext> vulkanContext;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Input> input;
};
