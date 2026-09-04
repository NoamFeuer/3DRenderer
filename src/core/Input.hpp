#pragma once

#include "Window.hpp"

enum class Key {
    W, A, S, D,
    Space, LeftShift,
    Escape,
    Up, Down, Left, Right
};

class Input {
public:
    explicit Input(Window& window);

    void update();

    bool isKeyDown(Key key) const;

    float getMouseDeltaX() const { return mouseDeltaX; }
    float getMouseDeltaY() const { return mouseDeltaY; }

    void setCursorCaptured(bool captured);

private:
    GLFWwindow* handle;
    double lastX = 0.0;
    double lastY = 0.0;
    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;
    bool firstUpdate = true;

    static int toGlfwKey(Key key);
};
