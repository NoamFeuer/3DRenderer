#include "Input.hpp"

Input::Input(Window& window) : handle(window.getHandle()) {
    setCursorCaptured(true);

    double x, y;
    glfwGetCursorPos(handle, &x, &y);
    lastX = x;
    lastY = y;
}

void Input::update() {
    double x, y;
    glfwGetCursorPos(handle, &x, &y);

    if (firstUpdate) {
        // Avoid a large jump on the very first frame, before lastX/lastY
        // have a meaningful previous value to compare against.
        lastX = x;
        lastY = y;
        firstUpdate = false;
    }

    mouseDeltaX = static_cast<float>(x - lastX);
    mouseDeltaY = static_cast<float>(y - lastY);
    lastX = x;
    lastY = y;
}

bool Input::isKeyDown(Key key) const {
    return glfwGetKey(handle, toGlfwKey(key)) == GLFW_PRESS;
}

void Input::setCursorCaptured(bool captured) {
    glfwSetInputMode(handle, GLFW_CURSOR, captured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

int Input::toGlfwKey(Key key) {
    switch (key) {
        case Key::W:          return GLFW_KEY_W;
        case Key::A:          return GLFW_KEY_A;
        case Key::S:          return GLFW_KEY_S;
        case Key::D:          return GLFW_KEY_D;
        case Key::Space:      return GLFW_KEY_SPACE;
        case Key::LeftShift:  return GLFW_KEY_LEFT_SHIFT;
        case Key::Escape:     return GLFW_KEY_ESCAPE;
        case Key::Up:         return GLFW_KEY_UP;
        case Key::Down:       return GLFW_KEY_DOWN;
        case Key::Left:       return GLFW_KEY_LEFT;
        case Key::Right:      return GLFW_KEY_RIGHT;
    }
    return GLFW_KEY_UNKNOWN;
}
