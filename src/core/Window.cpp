#include "Window.hpp"

#include <iostream>
#include <stdexcept>

void Window::errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << "\n";
}

Window::Window(int width, int height, const std::string& title) : width(width), height(height) {
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW!");

    std::cout << "GLFW version: " << glfwGetVersionString() << "\n";

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window!");
    }

    // Store `this` so the static GLFW callback can reach back into the Window instance that owns the handle it fired on.
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Window::framebufferResizeCallback(GLFWwindow* glfwWindow, int newWidth, int newHeight) {
    auto self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    self->framebufferResized = true;
    self->width = newWidth;
    self->height = newHeight;
}

bool Window::consumeFramebufferResized() {
    bool wasResized = framebufferResized;
    framebufferResized = false;
    return wasResized;
}

Window::~Window() {
    if (window)
        glfwDestroyWindow(window);

    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::pollEvents() const {
    glfwPollEvents();
}