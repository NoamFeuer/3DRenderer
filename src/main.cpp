#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>

#include "rendering/VulkanContext.hpp"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

int main() {
    glfwSetErrorCallback([](int error, const char* desc) {
        std::cerr << "GLFW Error " << error << ": " << desc << "\n";
    });

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    std::cout << "GLFW version: " << glfwGetVersionString() << "\n";

    // We're not using OpenGL, so tell GLFW not to create a GL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(
        WINDOW_WIDTH, WINDOW_HEIGHT,
        "My Engine", nullptr, nullptr
    );

    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    VulkanContext vulkanContext;
    try {
        vulkanContext.init(window);
    } catch (const std::exception& e) {
        std::cerr << "Vulkan error: " << e.what() << "\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        vulkanContext.drawFrame(window);
    }

    vulkanContext.waitIdle();
    vulkanContext.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
