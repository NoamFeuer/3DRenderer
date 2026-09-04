#pragma once

#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"

// A simple free-look camera. Position + yaw/pitch define where it is and
// where it's looking; getViewMatrix()/getProjectionMatrix() turn that into
// the matrices the shader actually needs.
class Camera {
public:
    Vect3 position{ 0.0f, 0.0f, 0.0f };
    float yaw = 0.0f;   // rotation around Y axis, radians
    float pitch = 0.0f; // rotation around X axis, radians

    float fov = 1.2217305f; // ~70 degrees, in radians (Mat4::projection expects radians)
    float nearPlane = 0.1f;
    float farPlane = 100.0f;

    Mat4 getViewMatrix() const;
    Mat4 getProjectionMatrix(float aspectRatio) const;

    // Movement-plane directions (yaw only — pitch doesn't tilt these,
    // matching typical FPS-style controls where looking up/down doesn't
    // change which way "forward" walks).
    Vect3 getForward() const;
    Vect3 getRight() const;
};
