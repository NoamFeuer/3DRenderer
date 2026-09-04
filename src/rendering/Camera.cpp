#include "Camera.hpp"
#include <cmath>

Mat4 Camera::getViewMatrix() const {
    // The camera's own world transform would be: translation(position) * rotationY(yaw) * rotationX(pitch)
    // (move into place, then yaw, then pitch). The view matrix is the INVERSE of that —
    // it transforms the world so the camera ends up at the origin looking down its own axis.
    //
    // Rotation matrices are orthogonal, so their inverse is just the rotation by the
    // negative angle, and translation's inverse is negation — so we can build the
    // inverse directly, in reverse order, without needing a general matrix inverse:
    //   inverse(T * Ry * Rx) = inverse(Rx) * inverse(Ry) * inverse(T)
    //                        = rotationX(-pitch) * rotationY(-yaw) * translation(-position)
    return Mat4::rotationX(-pitch) * Mat4::rotationY(-yaw) *
           Mat4::translation(-position.x, -position.y, -position.z);
}

Mat4 Camera::getProjectionMatrix(float aspectRatio) const {
    return Mat4::projection(fov, aspectRatio, nearPlane, farPlane);
}

Vect3 Camera::getForward() const {
    // The camera looks down its local -Z axis (objects with negative view-space
    // z are "in front" — the convention Mat4::projection was built for).
    // This is rotationY(yaw) applied to (0, 0, -1).
    return Vect3(-std::sin(yaw), 0.0f, -std::cos(yaw));
}

Vect3 Camera::getRight() const {
    // rotationY(yaw) applied to (1, 0, 0).
    return Vect3(std::cos(yaw), 0.0f, -std::sin(yaw));
}
