#include "Renderer.hpp"
#include <cmath>

void Renderer::beginFrame() {
    vertices.clear();
}

void Renderer::drawTriangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& color) {
    vertices.push_back({ p1, color });
    vertices.push_back({ p2, color });
    vertices.push_back({ p3, color });
}

void Renderer::drawRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4, const Vect3& color) {
    drawTriangle(p1, p2, p3, color);
    drawTriangle(p1, p3, p4, color);
}

void Renderer::drawCircle(const Vect3& center, float radius, const Vect3& color, int segments) {
    const float TWO_PI = 6.283185307179586f;

    for (int i = 0; i < segments; i++) {
        float angleA = TWO_PI * static_cast<float>(i) / static_cast<float>(segments);
        float angleB = TWO_PI * static_cast<float>(i + 1) / static_cast<float>(segments);

        Vect3 pointA(
            center.x + radius * std::cos(angleA),
            center.y + radius * std::sin(angleA),
            center.z
        );
        Vect3 pointB(
            center.x + radius * std::cos(angleB),
            center.y + radius * std::sin(angleB),
            center.z
        );

        drawTriangle(center, pointA, pointB, color);
    }
}

void Renderer::drawCube(const Vect3& center, const Vect3& color, const Mat4& rotation, const Vect3& scale) {
    float h = 0.5f;

    std::vector<Vect3> localCorners = {
        Vect3(-h, -h, -h), Vect3( h, -h, -h), Vect3( h,  h, -h), Vect3(-h,  h, -h),
        Vect3(-h, -h,  h), Vect3( h, -h,  h), Vect3( h,  h,  h), Vect3(-h,  h,  h)
    };

    for (Vect3& corner : localCorners) {
        corner *= scale;
        corner = rotation * corner;
        corner += center;
    }

    Vect3 p0 = localCorners[0];
    Vect3 p1 = localCorners[1];
    Vect3 p2 = localCorners[2];
    Vect3 p3 = localCorners[3];
    Vect3 p4 = localCorners[4];
    Vect3 p5 = localCorners[5];
    Vect3 p6 = localCorners[6];
    Vect3 p7 = localCorners[7];

    drawRectangle(p0, p1, p2, p3, color);
    drawRectangle(p5, p4, p7, p6, color);
    drawRectangle(p4, p0, p3, p7, color);
    drawRectangle(p1, p5, p6, p2, color);
    drawRectangle(p3, p2, p6, p7, color);
    drawRectangle(p4, p5, p1, p0, color); 
}
