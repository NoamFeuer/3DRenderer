#pragma once

#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"

class Camera {
public:
    enum class Type {
        Perspective,
        Orthographic
    };

    // Perspective (default) or Orthographic (2D). Switch freely between frames.
    Type type = Type::Perspective;

    Vect3 position{ 0.0f, 0.0f, 0.0f };
    float yaw = 0.0f; // rotation around Y axis, radians
    float pitch = 0.0f; // rotation around X axis, radians

    // Perspective settings. fov in radians.
    float fov = 1.2217305f;

    // Orthographic settings: vertical half-extent of the visible area in world
    // units. The horizontal extent is derived from the window aspect ratio.
    // In 2D mode position acts as the camera centre (scroll by moving it).
    float orthoHalfHeight = 5.0f;

    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    Mat4 getViewMatrix() const;
    Mat4 getProjectionMatrix(float aspectRatio) const;

    Vect3 getForward() const;
    Vect3 getRight() const;
};
