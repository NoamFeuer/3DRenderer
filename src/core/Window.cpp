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

    // We're not using OpenGL, so tell GLFW not to create a GL context.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window!");
    }
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
