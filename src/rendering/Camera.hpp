#pragma once

#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"

class Camera {
public:
    Vect3 position{ 0.0f, 0.0f, 0.0f };
    float yaw = 0.0f; // rotation around Y axis, radians
    float pitch = 0.0f; // rotation around X axis, radians

    float fov = 1.2217305f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    Mat4 getViewMatrix() const;
    Mat4 getProjectionMatrix(float aspectRatio) const;

    Vect3 getForward() const;
    Vect3 getRight() const;
};
