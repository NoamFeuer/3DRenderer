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
    // Draw two triangles to form a rectangle
    drawTriangle(p1, p2, p3, color);
    drawTriangle(p1, p3, p4, color);
}

void Renderer::drawCircle(const Vect3& center, float radius, const Vect3& color, int segments) {
    // Approximate the circle as a fan of triangles, all sharing the center point.
    // Walk around the circle in even angle steps, forming one triangle per step
    // between the current point, the next point, and the center.
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

void Renderer::drawCube(const Vect3& center, float size, const Vect3& color) {
    float h = size / 2.0f;

    // 8 corners of the cube.
    Vect3 p0 = center + Vect3(-h, -h, -h);
    Vect3 p1 = center + Vect3( h, -h, -h);
    Vect3 p2 = center + Vect3( h,  h, -h);
    Vect3 p3 = center + Vect3(-h,  h, -h);
    Vect3 p4 = center + Vect3(-h, -h,  h);
    Vect3 p5 = center + Vect3( h, -h,  h);
    Vect3 p6 = center + Vect3( h,  h,  h);
    Vect3 p7 = center + Vect3(-h,  h,  h);

    // 6 faces, 2 triangles each, via drawRectangle for readability.
    drawRectangle(p0, p1, p2, p3, color); // back  (-Z)
    drawRectangle(p5, p4, p7, p6, color); // front (+Z)
    drawRectangle(p4, p0, p3, p7, color); // left  (-X)
    drawRectangle(p1, p5, p6, p2, color); // right (+X)
    drawRectangle(p3, p2, p6, p7, color); // top   (+Y)
    drawRectangle(p4, p5, p1, p0, color); // bottom(-Y)
}
