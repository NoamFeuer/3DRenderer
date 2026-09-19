#include "Camera.hpp"
#include <cmath>

Mat4 Camera::getViewMatrix() const {
    return getOrientation().conjugate().toMatrix() *
           Mat4::translation(-position.x, -position.y, -position.z);
}

Quat Camera::getOrientation() const {
    return Quat::fromEuler(yaw, pitch, 0.0f);
}

Mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    if (type == Type::Orthographic) {
        float halfWidth = orthoHalfHeight * aspectRatio;
        Mat4 m = Mat4::ortho(-halfWidth, halfWidth, -orthoHalfHeight, orthoHalfHeight, nearPlane, farPlane);
        // Flip Y for Vulkan's y-down clip space (same reason as the
        // perspective projection in Mat4::projection).
        m.m[1][1] = -m.m[1][1];
        return m;
    }
    return Mat4::projection(fov, aspectRatio, nearPlane, farPlane);
}

Vect3 Camera::getForward() const {
    return Vect3(-std::sin(yaw), 0.0f, -std::cos(yaw));
}

Vect3 Camera::getRight() const {
    return Vect3(std::cos(yaw), 0.0f, -std::sin(yaw));
}

Vect3 Camera::getUp() const {
    return getOrientation().rotate(Vect3(0.0f, 1.0f, 0.0f));
}
