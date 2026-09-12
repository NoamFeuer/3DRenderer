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
    // updateCallback is called once per frame with the real frame delta time —
    // this is where a user of the library writes their own logic, reads input,
    // moves the camera, and issues draw calls via `renderer`.
    void run(const std::function<void(float deltaTime, Renderer& renderer, Camera& camera, Input& input)>& updateCallback = nullptr);

    // Fixed-timestep game loop, ideal for deterministic simulations and physics.
    // `fixedUpdate` runs exactly every `fixedDeltaTime` seconds (clamped so a slow
    // frame can't spiral out of control); `renderCallback` runs once per presented
    // frame with `alpha` (0..1) = how far the real time is between two fixed steps,
    // which you can use to interpolate visuals. Draw calls belong in `renderCallback`,
    // never in `fixedUpdate` (it may run several times per frame). State mutated by
    // `fixedUpdate` is copied to the draw code via values captured in the callbacks.
    void runFixed(
        float fixedDeltaTime,
        const std::function<void(float deltaTime, Renderer& renderer, Camera& camera, Input& input)>& fixedUpdate,
        const std::function<void(float alpha, Renderer& renderer, Camera& camera, Input& input)>& renderCallback = nullptr);

    // Loads a texture from disk into GPU memory and registers it with the
    // renderer. Returns the bindless index to pass to draw calls (e.g.
    // Renderer::drawTexturedCube). The texture stays alive until the Engine
    // is destroyed.
    int loadTexture(const std::string& path);

    // Loads a TTF/OTF font and rasterizes its printable ASCII range into a GPU
    // atlas at `pixelHeight`. Returns a pointer valid for the Engine's
    // lifetime; pass it to Renderer::drawText().
    Font* loadFont(const std::string& path, int pixelHeight = 64);

private:
    std::unique_ptr<Window> window;
    std::unique_ptr<VulkanContext> vulkanContext;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Input> input;
    std::vector<Texture> textures;
    std::vector<std::unique_ptr<Font>> fonts;
};
