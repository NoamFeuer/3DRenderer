#pragma once

#include <vector>

#include "Vertex.hpp"
#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"

class Renderer {
public:
    void beginFrame();

    void drawTriangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& color);
    void drawRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4, const Vect3& color);
    
    void drawCircle(const Vect3& center, float radius, const Vect3& color, int segments = 32);
    void drawCube(const Vect3& center, const Vect3& color,
              const Mat4& rotation = Mat4::identity(),
              const Vect3& scale = Vect3(1.0f, 1.0f, 1.0f));

    // Textured variants. `textureIndex` comes from VulkanContext::registerTexture();
    // pass -1 to sample no texture (drawColor used as-is).
    void drawTexturedRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4,
                          const Vect3& color, int textureIndex);
    void drawTexturedCube(const Vect3& center, const Vect3& color, int textureIndex,
                      const Mat4& rotation = Mat4::identity(),
                      const Vect3& scale = Vect3(1.0f, 1.0f, 1.0f));
    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<uint32_t>& getIndices() const { return indices; }

private:
    uint32_t pushUniqueVertex(const Vertex& vertex);
    void drawCubeWithTexture(const Vect3& center, const Vect3& color, int textureIndex,
                         const Mat4& rotation, const Vect3& scale);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};
