#pragma once

#include <cmath>
#include <algorithm>
#include <limits>

#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"
#include "Camera.hpp"

namespace ShadowUtils {
inline Mat4 lookAt(const Vect3& eye, const Vect3& target, const Vect3& worldUp) {
    Vect3 f = (target - eye).normalize();
    Vect3 s = f.cross(worldUp).normalize();
    if (s.lengthSquared() < 1e-6f)
        s = f.cross(Vect3(0.0f, 0.0f, 1.0f)).normalize();
    Vect3 u = s.cross(f);

    Mat4 m;
    m.m[0][0] = s.x;   m.m[0][1] = s.y;   m.m[0][2] = s.z;   m.m[0][3] = -s.dot(eye);
    m.m[1][0] = u.x;   m.m[1][1] = u.y;   m.m[1][2] = u.z;   m.m[1][3] = -u.dot(eye);
    m.m[2][0] = -f.x;  m.m[2][1] = -f.y;  m.m[2][2] = -f.z;  m.m[2][3] =  f.dot(eye);
    m.m[3][0] = 0.0f;  m.m[3][1] = 0.0f;  m.m[3][2] = 0.0f;  m.m[3][3] = 1.0f;
    return m;
}

inline Mat4 computeDirectionalShadowMatrix(const Camera& camera, float aspect, const Vect3& lightDir) {
    float cosPitch = std::cos(camera.pitch);
    float sinPitch = std::sin(camera.pitch);
    float cosYaw   = std::cos(camera.yaw);
    float sinYaw   = std::sin(camera.yaw);
    Vect3 fwd(-sinYaw * cosPitch, sinPitch, -cosYaw * cosPitch);

    Vect3 right = camera.getRight();
    Vect3 up = right.cross(fwd).normalize();

    float nh, nw, fh, fw;
    Vect3 nc, fc;
    if (camera.type == Camera::Type::Orthographic) {
        nh = camera.orthoHalfHeight;
        nw = nh * aspect;
        fh = nh;
        fw = nw;
        nc = camera.position + fwd * camera.nearPlane;
        fc = camera.position + fwd * camera.farPlane;
    }
    else {
        nh = camera.nearPlane * std::tan(camera.fov * 0.5f);
        nw = nh * aspect;
        fh = camera.farPlane * std::tan(camera.fov * 0.5f);
        fw = fh * aspect;
        nc = camera.position + fwd * camera.nearPlane;
        fc = camera.position + fwd * camera.farPlane;
    }

    Vect3 corners[8] = {
        nc - right * nw - up * nh,  nc + right * nw - up * nh,
        nc + right * nw + up * nh,  nc - right * nw + up * nh,
        fc - right * fw - up * fh,  fc + right * fw - up * fh,
        fc + right * fw + up * fh,  fc - right * fw + up * fh
    };

    Vect3 lightFwd = lightDir.normalize();
    Vect3 lightUpHint = (std::abs(lightFwd.y) < 0.999f) ? Vect3(0.0f, 1.0f, 0.0f) : Vect3(1.0f, 0.0f, 0.0f);

    Vect3 frustumCenter;
    for (const auto& c : corners)
        frustumCenter += c;
    frustumCenter *= (1.0f / 8.0f);

    Vect3 lightEye = frustumCenter - lightFwd * 200.0f;
    Mat4 lightView = lookAt(lightEye, frustumCenter, lightUpHint);

    float minX =  std::numeric_limits<float>::max();
    float maxX = -minX;
    float minY =  minX;
    float maxY =  maxX;
    float minZ =  minX;
    float maxZ =  maxX;

    for (const auto& c : corners) {
        Vect3 lp = lightView * c;
        if (lp.x < minX) minX = lp.x;
        if (lp.x > maxX) maxX = lp.x;
        if (lp.y < minY) minY = lp.y;
        if (lp.y > maxY) maxY = lp.y;
        if (lp.z < minZ) minZ = lp.z;
        if (lp.z > maxZ) maxZ = lp.z;
    }

    float margin = 15.0f;
    minX -= margin; maxX += margin;
    minY -= margin; maxY += margin;
    minZ -= 1.0f;
    maxZ += 1.0f;

    Mat4 lightProj = Mat4::ortho(minX, maxX, minY, maxY, -maxZ, -minZ);
    return lightProj * lightView;
}
}