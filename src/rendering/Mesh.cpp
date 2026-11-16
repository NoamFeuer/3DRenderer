#include "../third_party/tiny_gltf.h"

#include "Mesh.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>

// glTF stores images inside .glb containers and as sidecar files for .gltf.
// The engine's texture path is file-based (Texture::load), and Mesh carries no
// material payload, so geometry loads without textures here. The stub keeps
// tinygltf from failing on models that reference a baseColor texture.
static bool StubLoadImage(tinygltf::Image* image, const int image_idx,
                          std::string* err, std::string* warn,
                          int req_width, int req_height,
                          const unsigned char* bytes, int size, void* user_data) {
    (void)image_idx;
    (void)err;
    (void)warn;
    (void)req_width;
    (void)req_height;
    (void)bytes;
    (void)size;
    (void)user_data;

    if (image) {
        image->width = 1;
        image->height = 1;
        image->component = 4;
        image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
        image->image.assign(4, 255);
    }
    return true;
}

namespace {
    float twoPi = 6.283185307179586f;

    struct FaceVert {
        int position = -1;
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

    FaceVert parseFaceVertex(const std::string& token, int posCount, int uvCount, int normCount) {
        FaceVert fv;

        size_t slash1 = token.find('/');
        size_t slash2 = (slash1 == std::string::npos) ? std::string::npos : token.find('/', slash1 + 1);

        auto parseIndex = [](const std::string& s, int count) -> int {
            if (s.empty()) return -1;
            int idx = std::stoi(s);
            if (idx < 0) idx = count + idx + 1;
            return idx - 1;
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

    const uint32_t northPole = static_cast<uint32_t>(mesh.vertices.size());
    {
        VertexData v;
        v.position = Vect3(0.0f, radius, 0.0f);
        v.normal = Vect3(0.0f, 1.0f, 0.0f);
        v.u = 0.0f;
        v.v = 0.0f;
        mesh.vertices.push_back(v);
    }
    
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

    const uint32_t southPole = static_cast<uint32_t>(mesh.vertices.size());
    {
        VertexData v;
        v.position = Vect3(0.0f, -radius, 0.0f);
        v.normal = Vect3(0.0f, -1.0f, 0.0f);
        v.u = 0.0f;
        v.v = 1.0f;
        mesh.vertices.push_back(v);
    }

    for (int slice = 0; slice < slicesUsed; slice++) {
        int next = (slice + 1) % slicesUsed;
        mesh.indices.push_back(northPole);
        mesh.indices.push_back(ringIndex[1][next]);
        mesh.indices.push_back(ringIndex[1][slice]);
    }

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

Aabb Mesh::bounds() const {
    Aabb box;
    for (const auto& v : vertices)
        box.expand(v.position);
    return box;
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
    }

    if (positions.empty())
        throw std::runtime_error("Invalid OBJ file (no vertices): " + path);

    Mesh mesh;
    std::unordered_map<FaceVert, uint32_t, FaceVertHash> unique;

    for (const auto& face : faces) {
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

    bool hasAnyNormal = false;
    for (const auto& face : faces)
        for (const FaceVert& fv : face)
            if (fv.normal >= 0) { hasAnyNormal = true; break; }
    if (!hasAnyNormal)
        mesh.computeNormals();

    return mesh;
}

namespace {

// Number of components for a glTF accessor type.
int componentCount(int type) {
    switch (type) {
        case TINYGLTF_TYPE_SCALAR: return 1;
        case TINYGLTF_TYPE_VEC2:   return 2;
        case TINYGLTF_TYPE_VEC3:   return 3;
        case TINYGLTF_TYPE_VEC4:   return 4;
        case TINYGLTF_TYPE_MAT2:   return 4;
        case TINYGLTF_TYPE_MAT3:   return 9;
        case TINYGLTF_TYPE_MAT4:   return 16;
        default: return 0;
    }
}

// Size in bytes of one component of the given componentType.
size_t componentSize(int componentType) {
    switch (componentType) {
        case TINYGLTF_COMPONENT_TYPE_BYTE:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: return 1;
        case TINYGLTF_COMPONENT_TYPE_SHORT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: return 2;
        case TINYGLTF_COMPONENT_TYPE_INT:
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        case TINYGLTF_COMPONENT_TYPE_FLOAT: return 4;
        default: return 0;
    }
}

// Maps a glTF accessor element + component index to a float, honouring
// KHR_mesh_quantization (normalized and non-float integer attributes) and
// sparse accessors. Returns 0 for components that are absent.
class AccessorReader {
public:
    AccessorReader(const tinygltf::Model& model, const tinygltf::Accessor& accessor)
        : compType_(accessor.componentType),
          compSize_(componentSize(accessor.componentType)),
          numComponents_(componentCount(accessor.type)),
          normalized_(accessor.normalized),
          tightStride_(compSize_ * static_cast<size_t>(numComponents_)) {
        if (accessor.componentType == static_cast<int>(TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) - 1 ||
            accessor.type == static_cast<int>(TINYGLTF_TYPE_SCALAR) - 1 || compSize_ == 0 || numComponents_ == 0)
            return;

        stride_ = tightStride_;
        const unsigned char* base = nullptr;

        if (accessor.bufferView >= 0 &&
            static_cast<size_t>(accessor.bufferView) < model.bufferViews.size()) {
            const tinygltf::BufferView& view = model.bufferViews[static_cast<size_t>(accessor.bufferView)];
            if (view.byteStride > 0)
                stride_ = view.byteStride;
            if (view.buffer >= 0 && static_cast<size_t>(view.buffer) < model.buffers.size()) {
                const tinygltf::Buffer& buffer = model.buffers[static_cast<size_t>(view.buffer)];
                base = buffer.data.data() + view.byteOffset + accessor.byteOffset;
            }
        }

        const tinygltf::Accessor::Sparse& sparse = accessor.sparse;
        if (sparse.isSparse && sparse.count > 0) {
            // Sparse accessors carry only the overridden elements, so
            // materialise the full range into scratch storage.
            owned_.resize(static_cast<size_t>(accessor.count) * tightStride_, 0);
            if (base != nullptr) {
                for (size_t i = 0; i < accessor.count; i++)
                    std::memcpy(owned_.data() + i * tightStride_, base + i * stride_, tightStride_);
            }
            applySparse(model, accessor);
            base = owned_.data();
            stride_ = tightStride_;
        }

        data_ = base;
    }

    float get(size_t element, int component) const {
        if (data_ == nullptr || component >= numComponents_)
            return 0.0f;

        const unsigned char* p = data_ + element * stride_ + static_cast<size_t>(component) * compSize_;
        return decode(p);
    }

private:
    // Copies the sparse substitution list over the base elements.
    void applySparse(const tinygltf::Model& model, const tinygltf::Accessor& accessor) {
        const tinygltf::Accessor::Sparse& sparse = accessor.sparse;

        if (sparse.indices.bufferView < 0 || sparse.values.bufferView < 0)
            return;
        if (static_cast<size_t>(sparse.indices.bufferView) >= model.bufferViews.size() ||
            static_cast<size_t>(sparse.values.bufferView) >= model.bufferViews.size())
            return;

        const tinygltf::BufferView& idxView = model.bufferViews[static_cast<size_t>(sparse.indices.bufferView)];
        const tinygltf::BufferView& valView = model.bufferViews[static_cast<size_t>(sparse.values.bufferView)];
        if (idxView.buffer < 0 || valView.buffer < 0)
            return;

        const tinygltf::Buffer& idxBuffer = model.buffers[static_cast<size_t>(idxView.buffer)];
        const tinygltf::Buffer& valBuffer = model.buffers[static_cast<size_t>(valView.buffer)];

        const size_t idxCompSize = componentSize(sparse.indices.componentType);
        const size_t idxStride = idxCompSize;
        const size_t valStride = tightStride_;

        for (size_t i = 0; i < static_cast<size_t>(sparse.count); i++) {
            const unsigned char* idxPtr = idxBuffer.data.data() + idxView.byteOffset +
                                           sparse.indices.byteOffset + i * idxStride;

            size_t element = 0;
            switch (sparse.indices.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: element = *idxPtr; break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    std::memcpy(&element, idxPtr, sizeof(uint16_t));
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                    uint32_t v = 0;
                    std::memcpy(&v, idxPtr, sizeof(uint32_t));
                    element = v;
                    break;
                }
                default: continue;
            }

            if (element >= accessor.count)
                continue;

            const unsigned char* valPtr = valBuffer.data.data() + valView.byteOffset +
                                           sparse.values.byteOffset + i * valStride;
            std::memcpy(owned_.data() + element * tightStride_, valPtr, tightStride_);
        }
    }

    float decode(const unsigned char* p) const {
        switch (compType_) {
            case TINYGLTF_COMPONENT_TYPE_FLOAT: {
                float v;
                std::memcpy(&v, p, sizeof(v));
                return v;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: {
                const float v = static_cast<float>(*p);
                return normalized_ ? v / 255.0f : v;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
                uint16_t raw;
                std::memcpy(&raw, p, sizeof(raw));
                const float v = static_cast<float>(raw);
                return normalized_ ? v / 65535.0f : v;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
                uint32_t raw;
                std::memcpy(&raw, p, sizeof(raw));
                return static_cast<float>(raw);
            }
            case TINYGLTF_COMPONENT_TYPE_BYTE: {
                int8_t raw;
                std::memcpy(&raw, p, sizeof(raw));
                const float v = static_cast<float>(raw);
                return normalized_ ? std::max(v / 127.0f, -1.0f) : v;
            }
            case TINYGLTF_COMPONENT_TYPE_SHORT: {
                int16_t raw;
                std::memcpy(&raw, p, sizeof(raw));
                const float v = static_cast<float>(raw);
                return normalized_ ? std::max(v / 32767.0f, -1.0f) : v;
            }
            case TINYGLTF_COMPONENT_TYPE_INT: {
                int32_t raw;
                std::memcpy(&raw, p, sizeof(raw));
                return static_cast<float>(raw);
            }
            default:
                return 0.0f;
        }
    }

    int compType_ = static_cast<int>(TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) - 1;
    size_t compSize_ = 0;
    int numComponents_ = 0;
    bool normalized_ = false;
    size_t tightStride_ = 0;
    size_t stride_ = 0;
    const unsigned char* data_ = nullptr;
    std::vector<unsigned char> owned_;
};

// Row-major local transform for a node, from either `matrix` or TRS.
Mat4 nodeLocalMatrix(const tinygltf::Node& node) {
    if (node.matrix.size() == 16) {
        // glTF matrices are column-major; transcribe into row-major.
        Mat4 result;
        for (int col = 0; col < 4; col++)
            for (int row = 0; row < 4; row++)
                result.m[row][col] = static_cast<float>(node.matrix[static_cast<size_t>(col) * 4 + static_cast<size_t>(row)]);
        return result;
    }

    Mat4 t = node.translation.size() == 3
        ? Mat4::translation(node.translation[0], node.translation[1], node.translation[2])
        : Mat4::identity();

    Mat4 r = Mat4::identity();
    if (node.rotation.size() == 4) {
        const float x = static_cast<float>(node.rotation[0]);
        const float y = static_cast<float>(node.rotation[1]);
        const float z = static_cast<float>(node.rotation[2]);
        const float w = static_cast<float>(node.rotation[3]);

        Mat4 q;
        q.m[0][0] = 1.0f - 2.0f * (y * y + z * z);
        q.m[0][1] = 2.0f * (x * y + z * w);
        q.m[0][2] = 2.0f * (x * z - y * w);
        q.m[1][0] = 2.0f * (x * y - z * w);
        q.m[1][1] = 1.0f - 2.0f * (x * x + z * z);
        q.m[1][2] = 2.0f * (y * z + x * w);
        q.m[2][0] = 2.0f * (x * z + y * w);
        q.m[2][1] = 2.0f * (y * z - x * w);
        q.m[2][2] = 1.0f - 2.0f * (x * x + y * y);
        q.m[3][3] = 1.0f;
        r = q;
    }

    Mat4 s = node.scale.size() == 3
        ? Mat4::scale(node.scale[0], node.scale[1], node.scale[2])
        : Mat4::identity();

    return t * r * s;
}

} // namespace

Mesh Mesh::loadGltf(const std::string& path) {
    tinygltf::TinyGLTF loader;
    loader.SetImageLoader(StubLoadImage, nullptr);

    tinygltf::Model model;
    std::string err;
    std::string warn;
    const bool isBinary = path.size() >= 4 &&
        (path.compare(path.size() - 4, 4, ".glb") == 0 ||
         path.compare(path.size() - 4, 4, ".GLB") == 0);

    const bool ok = isBinary
        ? loader.LoadBinaryFromFile(&model, &err, &warn, path)
        : loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty())
        std::cerr << "glTF warning: " << warn << "\n";
    if (!ok)
        throw std::runtime_error("Failed to load glTF: " + path + " (" + err + ")");
    if (model.meshes.empty())
        throw std::runtime_error("glTF contains no meshes: " + path);

    // Flatten the scene graph into a list of (primitive, world transform)
    // pairs. Mesh is a single geometry buffer with no per-node transform, so
    // node hierarchies are baked into the vertices.
    struct DrawItem {
        const tinygltf::Primitive* primitive;
        Mat4 transform;
        Mat4 normalTransform;
    };
    std::vector<DrawItem> items;

    std::function<void(int, const Mat4&)> visitNode = [&](int nodeIndex, const Mat4& parent) {
        const tinygltf::Node& node = model.nodes[static_cast<size_t>(nodeIndex)];
        const Mat4 world = parent * nodeLocalMatrix(node);

        if (node.mesh >= 0 && static_cast<size_t>(node.mesh) < model.meshes.size()) {
            for (const tinygltf::Primitive& prim : model.meshes[static_cast<size_t>(node.mesh)].primitives) {
                // Normals need the inverse-transpose; for rigid+uniform-scale
                // transforms the upper 3x3 suffices.
                Mat4 normal = world;
                normal.m[0][3] = 0.0f;
                normal.m[1][3] = 0.0f;
                normal.m[2][3] = 0.0f;
                items.push_back(DrawItem{ &prim, world, normal });
            }
        }

        for (int child : node.children)
            visitNode(child, world);
    };

    if (!model.scenes.empty()) {
        const tinygltf::Scene& scene = model.scenes[model.defaultScene < 0 ? 0 : static_cast<size_t>(model.defaultScene)];
        for (int root : scene.nodes)
            visitNode(root, Mat4::identity());
    } else {
        for (size_t i = 0; i < model.nodes.size(); i++)
            visitNode(static_cast<int>(i), Mat4::identity());
    }

    if (items.empty())
        throw std::runtime_error("glTF contains no drawable primitives: " + path);

    Mesh mesh;
    bool anyNormals = false;

    for (const DrawItem& item : items) {
        const tinygltf::Primitive& prim = *item.primitive;

        const auto posIt = prim.attributes.find("POSITION");
        if (posIt == prim.attributes.end())
            continue;
        if (posIt->second < 0 || static_cast<size_t>(posIt->second) >= model.accessors.size())
            continue;

        const tinygltf::Accessor& posAccessor = model.accessors[static_cast<size_t>(posIt->second)];
        const AccessorReader positions(model, posAccessor);

        std::optional<AccessorReader> normals;
        const auto nrmIt = prim.attributes.find("NORMAL");
        if (nrmIt != prim.attributes.end() && nrmIt->second >= 0 &&
            static_cast<size_t>(nrmIt->second) < model.accessors.size()) {
            normals.emplace(model, model.accessors[static_cast<size_t>(nrmIt->second)]);
            anyNormals = true;
        }

        std::optional<AccessorReader> uvs;
        const auto uvIt = prim.attributes.find("TEXCOORD_0");
        if (uvIt != prim.attributes.end() && uvIt->second >= 0 &&
            static_cast<size_t>(uvIt->second) < model.accessors.size()) {
            uvs.emplace(model, model.accessors[static_cast<size_t>(uvIt->second)]);
        }

        // Primitive-local vertex 0 becomes this global vertex.
        const uint32_t vertexBase = static_cast<uint32_t>(mesh.vertices.size());

        for (size_t i = 0; i < posAccessor.count; i++) {
            const Vect3 position = item.transform * Vect3(positions.get(i, 0), positions.get(i, 1), positions.get(i, 2));

            Vect3 normal{ 0.0f, 0.0f, 1.0f };
            if (normals) {
                const Vect3 n(normals->get(i, 0), normals->get(i, 1), normals->get(i, 2));
                normal = Vect3(
                    item.normalTransform.m[0][0] * n.x + item.normalTransform.m[1][0] * n.y + item.normalTransform.m[2][0] * n.z,
                    item.normalTransform.m[0][1] * n.x + item.normalTransform.m[1][1] * n.y + item.normalTransform.m[2][1] * n.z,
                    item.normalTransform.m[0][2] * n.x + item.normalTransform.m[1][2] * n.y + item.normalTransform.m[2][2] * n.z);
                if (normal.x != 0.0f || normal.y != 0.0f || normal.z != 0.0f)
                    normal = normal.normalize();
            }

            VertexData vertex;
            vertex.position = position;
            vertex.normal = normal;
            if (uvs) {
                vertex.u = uvs->get(i, 0);
                vertex.v = uvs->get(i, 1);
            }
            mesh.vertices.push_back(vertex);
        }

        if (prim.indices >= 0 && static_cast<size_t>(prim.indices) < model.accessors.size()) {
            const tinygltf::Accessor& idxAccessor = model.accessors[static_cast<size_t>(prim.indices)];
            const AccessorReader indexReader(model, idxAccessor);
            for (size_t i = 0; i < idxAccessor.count; i++)
                mesh.indices.push_back(vertexBase + static_cast<uint32_t>(indexReader.get(i, 0)));
        } else {
            // Non-indexed primitive: generate a trivial index list.
            for (uint32_t i = 0; i < posAccessor.count; i++)
                mesh.indices.push_back(vertexBase + i);
        }
    }

    if (mesh.vertices.empty())
        throw std::runtime_error("glTF has no POSITION data: " + path);
    if (!anyNormals)
        mesh.computeNormals();

    return mesh;
}

Mesh Mesh::loadModel(const std::string& path) {
    std::string extension;
    const size_t dot = path.find_last_of('.');
    if (dot != std::string::npos) {
        extension = path.substr(dot);
        for (char& c : extension)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    if (extension == ".gltf" || extension == ".glb")
        return loadGltf(path);
    if (extension == ".obj")
        return loadObj(path);

    // Unknown extension: try glTF first, then fall back to OBJ.
    try {
        return loadGltf(path);
    } catch (const std::exception&) {
        return loadObj(path);
    }
}
