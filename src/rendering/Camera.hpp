#pragma once

#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"
#include "../math/Quat.hpp"

class Camera {
public:
    enum class Type {
        Perspective,
        Orthographic
    };

    Type type = Type::Perspective;

    Vect3 position{ 0.0f, 0.0f, 0.0f };
    float yaw = 0.0f;
    float pitch = 0.0f;
    float fov = 1.2217305f;

    float orthoHalfHeight = 5.0f;

    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    Mat4 getViewMatrix() const;
    Mat4 getProjectionMatrix(float aspectRatio) const;

    // World-space orientation of the camera (yaw around Y, then pitch around X).
    Quat getOrientation() const;

    Vect3 getForward() const;
    Vect3 getRight() const;
    Vect3 getUp() const;
};
