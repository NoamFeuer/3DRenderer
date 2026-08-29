#include "Renderer.hpp"

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
