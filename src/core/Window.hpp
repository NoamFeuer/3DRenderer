#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    // Not copyable: this owns a GLFW handle, copying would double-free it.
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool shouldClose() const;
    void pollEvents() const;

    GLFWwindow* getHandle() const { return window; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

private:
    GLFWwindow* window = nullptr;
    int width;
    int height;

    static void errorCallback(int error, const char* description);
};
