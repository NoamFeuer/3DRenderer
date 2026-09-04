#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const;
    void pollEvents() const;

    GLFWwindow* getHandle() const { return window; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // Returns true exactly once after a resize occurred, then resets - call this once per frame to check whether the swapchain needs rebuilding.
    bool consumeFramebufferResized();

private:
    GLFWwindow* window = nullptr;
    int width;
    int height;
    bool framebufferResized = false;

    static void errorCallback(int error, const char* description);
    static void framebufferResizeCallback(GLFWwindow* glfwWindow, int newWidth, int newHeight);
};