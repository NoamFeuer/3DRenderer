#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../math/Vect3.hpp"

class Mesh {
public:
    struct VertexData {
        Vect3 position;
        Vect3 normal{ 0.0f, 0.0f, 1.0f };
        float u = 0.0f;
        float v = 0.0f;
    };

    std::vector<VertexData> vertices;
    std::vector<uint32_t> indices;

    static Mesh createCube();
    static Mesh createSphere(float radius = 0.5f, int slices = 24, int stacks = 14);
    static Mesh createPlane(float width = 10.0f, float depth = 10.0f, int subdivisions = 1);
    static Mesh loadObj(const std::string& path);

    void computeNormals();
};