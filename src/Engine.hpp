#pragma once

#include <string>
#include <functional>
#include <memory>

#include "core/Window.hpp"
#include "rendering/VulkanContext.hpp"

// Engine is the public entry point to the library.
class Engine {
public:
    Engine(int width, int height, const std::string& title);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    // Runs the main loop until the window is closed.
    // updateCallback is called once per frame, before rendering — this is
    // where a user of the library would put their own per-frame logic.
    void run(const std::function<void(float deltaTime)>& updateCallback = nullptr);

private:
    std::unique_ptr<Window> window;
    std::unique_ptr<VulkanContext> vulkanContext;
};
