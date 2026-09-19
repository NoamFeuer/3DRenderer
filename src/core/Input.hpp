#pragma once

#include "Window.hpp"

enum class Key {
    W, A, S, D,
    E, Q,
    R, P,
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

    // Cursor position in pixels, origin at the top-left of the window.
    float getMouseX() const { return mouseX; }
    float getMouseY() const { return mouseY; }

    // True for a single frame when the left mouse button is pressed down.
    bool isMouseButtonPressed() const { return mouseButtonPressed; }

    void setCursorCaptured(bool captured);
    bool isCursorCaptured() const { return cursorCaptured; }

private:
    GLFWwindow* handle;
    double lastX = 0.0;
    double lastY = 0.0;
    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    bool mouseButtonDownPrev = false;
    bool mouseButtonPressed = false;
    bool firstUpdate = true;
    bool cursorCaptured = true;

    static int toGlfwKey(Key key);
};
