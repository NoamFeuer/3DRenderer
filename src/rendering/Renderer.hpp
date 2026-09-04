#pragma once

#include <vector>

#include "Vertex.hpp"
#include "../math/Vect3.hpp"

// Internally this just accumulates Vertex data each frame;
// Engine hands that data to VulkanContext, which uploads it and issues the actual draw calls.
class Renderer {
public:
    // Called once per frame, before the user's update callback runs.
    void beginFrame();

    // Draws a solid-colored triangle. All three corners share one color, per-vertex coloring can be added later if needed.
    void drawTriangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& color);
    void drawRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4, const Vect3& color);

    // Draws a solid-colored circle as a fan of triangles around the center.
    // segments controls smoothness — higher = rounder, more triangles.
    void drawCircle(const Vect3& center, float radius, const Vect3& color, int segments = 32);

    // Draws a solid-colored axis-aligned cube, centered at `center`,
    // `size` units along each edge.
    void drawCube(const Vect3& center, float size, const Vect3& color);

    const std::vector<Vertex>& getVertices() const { return vertices; }

private:
    std::vector<Vertex> vertices;
};
