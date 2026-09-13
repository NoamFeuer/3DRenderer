#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../math/Vect3.hpp"

// A CPU-side triangle mesh: unique vertices (position/normal/UV) plus an index
// list. Load from a Wavefront .obj file or generate primitives on the fly,
// then hand it to Renderer::drawMesh(). A Mesh is plain data — it can be
// created anywhere and drawn any number of times.
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

    // ---- Generators ----------------------------------------------------

    // Unit cube from -0.5..0.5, one face per side (faceted normals).
    static Mesh createCube();

    // UV sphere of the given radius centred at the origin.
    static Mesh createSphere(float radius = 0.5f, int slices = 24, int stacks = 14);

    // Flat XZ grid of the given width/depth centred at the origin (normal +Y),
    // useful as a ground plane / terrain base.
    static Mesh createPlane(float width = 10.0f, float depth = 10.0f, int subdivisions = 1);

    // ---- Loaders -------------------------------------------------------

    // Loads a Wavefront .obj (v / vn / vt / f). Missing normals are computed
    // by averaging face normals; missing UVs default to (0,0). Throws
    // std::runtime_error on unreadable/invalid files.
    static Mesh loadObj(const std::string& path);

    // Reorders the vertex/index data so the mesh has normals at every corner.
    // Called automatically after loading when a file lacks `vn` lines.
    void computeNormals();
};