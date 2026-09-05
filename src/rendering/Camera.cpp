#include "Camera.hpp"
#include <cmath>

Mat4 Camera::getViewMatrix() const {

    return Mat4::rotationX(-pitch) * Mat4::rotationY(-yaw) *
           Mat4::translation(-position.x, -position.y, -position.z);
}

Mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return Mat4::projection(fov, aspectRatio, nearPlane, farPlane);
}

Vect3 Camera::getForward() const {
    return Vect3(-std::sin(yaw), 0.0f, -std::cos(yaw));
}

Vect3 Camera::getRight() const {
    return Vect3(std::cos(yaw), 0.0f, -std::sin(yaw));
}
