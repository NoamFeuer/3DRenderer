#include "Mesh.hpp"

#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {
    float twoPi = 6.283185307179586f;

    struct FaceVert {
        int position = -1; // 0-based index into positions
        int uv = -1;
        int normal = -1;
    };

    bool operator==(const FaceVert& a, const FaceVert& b) {
        return a.position == b.position && a.uv == b.uv && a.normal == b.normal;
    }

    struct FaceVertHash {
        size_t operator()(const FaceVert& f) const {
            size_t h1 = std::hash<int>{}(f.position);
            size_t h2 = std::hash<int>{}(f.uv);
            size_t h3 = std::hash<int>{}(f.normal);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    // Parses "v/vt/vn", "v//vn", or "v" into resolved (0-based) indices.
    FaceVert parseFaceVertex(const std::string& token, int posCount, int uvCount, int normCount) {
        FaceVert fv;

        // Explicit 1-based positive indices are the common case; fall back to
        // a general parse that also handles negatives.
        size_t slash1 = token.find('/');
        size_t slash2 = (slash1 == std::string::npos) ? std::string::npos : token.find('/', slash1 + 1);

        auto parseIndex = [](const std::string& s, int count) -> int {
            if (s.empty()) return -1;
            int idx = std::stoi(s);
            if (idx < 0) idx = count + idx + 1; // negative = relative to end
            return idx - 1;                     // OBJ is 1-based
        };

        fv.position = parseIndex(token.substr(0, slash1), posCount);

        std::string uvTok, nTok;
        if (slash1 != std::string::npos) {
            size_t uvEnd = (slash2 == std::string::npos) ? token.size() : slash2;
            uvTok = token.substr(slash1 + 1, uvEnd - slash1 - 1);
            if (slash2 != std::string::npos)
                nTok = token.substr(slash2 + 1);
        }

        if (!uvTok.empty()) fv.uv = parseIndex(uvTok, uvCount);
        if (!nTok.empty()) fv.normal = parseIndex(nTok, normCount);

        return fv;
    }
}

Mesh Mesh::createCube() {
    Mesh mesh;

    // 6 faces, 4 unique vertices each, outward normals, full-face UVs.
    const float h = 0.5f;
    const struct {
        Vect3 v[4];
        Vect3 n;
    } faces[6] = {
        { { { -h, -h, -h }, { h, -h, -h }, { h,  h, -h }, { -h,  h, -h } }, { 0, 0, -1 } },
        { { {  h, -h,  h }, { -h, -h,  h }, { -h,  h,  h }, {  h,  h,  h } }, { 0, 0,  1 } },
        { { { -h, -h,  h }, { -h, -h, -h }, { -h,  h, -h }, { -h,  h,  h } }, { -1, 0, 0 } },
        { { {  h, -h, -h }, {  h, -h,  h }, {  h,  h,  h }, {  h,  h, -h } }, { 1, 0, 0 } },
        { { { -h,  h, -h }, {  h,  h, -h }, {  h,  h,  h }, { -h,  h,  h } }, { 0, 1, 0 } },
        { { { -h, -h,  h }, {  h, -h,  h }, {  h, -h, -h }, { -h, -h, -h } }, { 0, -1, 0 } }
    };

    for (const auto& face : faces) {
        const Vect3 uv[4] = { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 } };
        uint32_t base = static_cast<uint32_t>(mesh.vertices.size());
        for (int i = 0; i < 4; i++) {
            VertexData v;
            v.position = face.v[i];
            v.normal = face.n;
            v.u = uv[i].x;
            v.v = uv[i].y;
            mesh.vertices.push_back(v);
        }
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 3);
    }

    return mesh;
}

Mesh Mesh::createSphere(float radius, int slices, int stacks) {
    Mesh mesh;

    const int stacksUsed = stacks < 3 ? 3 : stacks;
    const int slicesUsed = slices < 3 ? 3 : slices;

    // Top cap at north pole.
    const uint32_t northPole = static_cast<uint32_t>(mesh.vertices.size());
    {
        VertexData v;
        v.position = Vect3(0.0f, radius, 0.0f);
        v.normal = Vect3(0.0f, 1.0f, 0.0f);
        v.u = 0.0f;
        v.v = 0.0f;
        mesh.vertices.push_back(v);
    }

    // Rings of vertices.
    std::vector<std::vector<uint32_t>> ringIndex(stacksUsed + 1);
    for (int stack = 1; stack <= stacksUsed; stack++) {
        const float phi = twoPi * static_cast<float>(stack) / static_cast<float>(stacksUsed + 1);
        float y = std::cos(phi);
        float r = std::sin(phi);
        for (int slice = 0; slice < slicesUsed; slice++) {
            const float theta = twoPi * static_cast<float>(slice) / static_cast<float>(slicesUsed);
            float x = std::cos(theta) * r;
            float z = std::sin(theta) * r;

            VertexData v;
            v.position = Vect3(x * radius, y * radius, z * radius);
            v.normal = Vect3(x, y, z);
            v.u = static_cast<float>(slice) / static_cast<float>(slicesUsed);
            v.v = static_cast<float>(stack) / static_cast<float>(stacksUsed + 1);
            ringIndex[stack].push_back(static_cast<uint32_t>(mesh.vertices.size()));
            mesh.vertices.push_back(v);
        }
    }

    // Bottom cap at south pole.
    const uint32_t southPole = static_cast<uint32_t>(mesh.vertices.size());
    {
        VertexData v;
        v.position = Vect3(0.0f, -radius, 0.0f);
        v.normal = Vect3(0.0f, -1.0f, 0.0f);
        v.u = 0.0f;
        v.v = 1.0f;
        mesh.vertices.push_back(v);
    }

    // North cap triangles (winding outward: face CCW when viewed from outside).
    for (int slice = 0; slice < slicesUsed; slice++) {
        int next = (slice + 1) % slicesUsed;
        mesh.indices.push_back(northPole);
        mesh.indices.push_back(ringIndex[1][next]);
        mesh.indices.push_back(ringIndex[1][slice]);
    }

    // Middle ring quads.
    for (int stack = 1; stack < stacksUsed; stack++) {
        for (int slice = 0; slice < slicesUsed; slice++) {
            int next = (slice + 1) % slicesUsed;
            uint32_t a = ringIndex[stack][slice];
            uint32_t b = ringIndex[stack][next];
            uint32_t c = ringIndex[stack + 1][next];
            uint32_t d = ringIndex[stack + 1][slice];

            mesh.indices.push_back(a); mesh.indices.push_back(b); mesh.indices.push_back(c);
            mesh.indices.push_back(a); mesh.indices.push_back(c); mesh.indices.push_back(d);
        }
    }

    // South cap triangles.
    for (int slice = 0; slice < slicesUsed; slice++) {
        int next = (slice + 1) % slicesUsed;
        mesh.indices.push_back(ringIndex[stacksUsed][slice]);
        mesh.indices.push_back(ringIndex[stacksUsed][next]);
        mesh.indices.push_back(southPole);
    }

    return mesh;
}

Mesh Mesh::createPlane(float width, float depth, int subdivisions) {
    Mesh mesh;

    const int seg = subdivisions < 1 ? 1 : subdivisions;
    const int vertsPerRow = seg + 1;

    auto vertexIndex = [&](int row, int col) -> uint32_t {
        return static_cast<uint32_t>(row * vertsPerRow + col);
    };

    for (int row = 0; row <= seg; row++) {
        for (int col = 0; col <= seg; col++) {
            float tx = static_cast<float>(col) / static_cast<float>(seg);
            float tz = static_cast<float>(row) / static_cast<float>(seg);

            VertexData v;
            v.position = Vect3((tx - 0.5f) * width, 0.0f, (tz - 0.5f) * depth);
            v.normal = Vect3(0.0f, 1.0f, 0.0f);
            v.u = tx;
            v.v = tz;
            mesh.vertices.push_back(v);
        }
    }

    for (int row = 0; row < seg; row++) {
        for (int col = 0; col < seg; col++) {
            uint32_t a = vertexIndex(row, col);
            uint32_t b = vertexIndex(row, col + 1);
            uint32_t c = vertexIndex(row + 1, col + 1);
            uint32_t d = vertexIndex(row + 1, col);

            mesh.indices.push_back(a); mesh.indices.push_back(b); mesh.indices.push_back(c);
            mesh.indices.push_back(a); mesh.indices.push_back(c); mesh.indices.push_back(d);
        }
    }

    return mesh;
}

void Mesh::computeNormals() {
    std::vector<Vect3> sums(vertices.size(), Vect3(0.0f, 0.0f, 0.0f));
    for (size_t t = 0; t + 2 < indices.size(); t += 3) {
        const VertexData& a = vertices[indices[t]];
        const VertexData& b = vertices[indices[t + 1]];
        const VertexData& c = vertices[indices[t + 2]];

        Vect3 edge1 = b.position - a.position;
        Vect3 edge2 = c.position - a.position;
        Vect3 faceNormal = edge1.cross(edge2);

        sums[indices[t]]     += faceNormal;
        sums[indices[t + 1]] += faceNormal;
        sums[indices[t + 2]] += faceNormal;
    }

    for (size_t i = 0; i < vertices.size(); i++) {
        Vect3 n = sums[i];
        float len = n.length();
        vertices[i].normal = (len > 1e-6f) ? n.normalize() : Vect3(0.0f, 1.0f, 0.0f);
    }
}

Mesh Mesh::loadObj(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Failed to open OBJ file: " + path);

    std::vector<Vect3> positions;
    std::vector<Vect3> normals;
    std::vector<std::pair<float, float>> uvs;
    std::vector<std::vector<FaceVert>> faces;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::string kind;
        ss >> kind;

        if (kind == "v") {
            float x, y, z;
            if (ss >> x >> y >> z)
                positions.emplace_back(x, y, z);
        }
        else if (kind == "vn") {
            float x, y, z;
            if (ss >> x >> y >> z)
                normals.emplace_back(x, y, z);
        }
        else if (kind == "vt") {
            float u = 0.0f, v = 0.0f;
            if (ss >> u)
                ss >> v;
            uvs.emplace_back(u, v);
        }
        else if (kind == "f") {
            std::vector<FaceVert> face;
            std::string token;
            while (ss >> token) {
                face.push_back(parseFaceVertex(
                    token,
                    static_cast<int>(positions.size()),
                    static_cast<int>(uvs.size()),
                    static_cast<int>(normals.size())));
            }
            if (face.size() >= 3)
                faces.push_back(face);
        }
        // ignore o/g/usemtl/mtllib/s/lights: not needed for geometry
    }

    if (positions.empty())
        throw std::runtime_error("Invalid OBJ file (no vertices): " + path);

    Mesh mesh;
    std::unordered_map<FaceVert, uint32_t, FaceVertHash> unique;

    for (const auto& face : faces) {
        // Fan triangulation for polygons / quads.
        for (size_t i = 1; i + 1 < face.size(); i++) {
            const FaceVert tris[3] = { face[0], face[i], face[i + 1] };
            for (const FaceVert& fv : tris) {
                auto it = unique.find(fv);
                if (it != unique.end()) {
                    mesh.indices.push_back(it->second);
                    continue;
                }

                VertexData v;
                v.position = positions[fv.position];
                if (fv.uv >= 0) {
                    v.u = uvs[static_cast<size_t>(fv.uv)].first;
                    v.v = uvs[static_cast<size_t>(fv.uv)].second;
                }
                if (fv.normal >= 0)
                    v.normal = normals[static_cast<size_t>(fv.normal)];

                uint32_t newIndex = static_cast<uint32_t>(mesh.vertices.size());
                unique.emplace(fv, newIndex);
                mesh.vertices.push_back(v);
                mesh.indices.push_back(newIndex);
            }
        }
    }

    // Smooth-shade faces when the file carried no normals.
    bool hasAnyNormal = false;
    for (const auto& face : faces)
        for (const FaceVert& fv : face)
            if (fv.normal >= 0) { hasAnyNormal = true; break; }
    if (!hasAnyNormal)
        mesh.computeNormals();

    return mesh;
}