#pragma once

#include <string>
#include <vector>

#include "Vertex.hpp"
#include "Material.hpp"
#include "Light.hpp"
#include "GpuData.hpp"
#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"

class Font;
class Mesh;

inline constexpr float HUD_Z = -0.1f;

class Renderer {
public:
    static constexpr uint32_t MAX_MODELS = 4096;

    LightingState lighting;

    Renderer();

    void beginFrame();
    void drawTriangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& color);
    void drawRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4, const Vect3& color);
    void drawCircle(const Vect3& center, float radius, const Vect3& color, int segments = 32);
    void drawCube(const Vect3& center, const Vect3& color, const Mat4& rotation = Mat4::identity(), const Vect3& scale = Vect3(1.0f, 1.0f, 1.0f), const Material& material = Material());
    // Full-model-matrix variants. These accept an arbitrary world transform
    // (e.g. a scene-graph world matrix) instead of assembling one from
    // center/rotation/scale.
    void drawCube(const Mat4& model, const Vect3& color, const Material& material = Material(), int textureIndex = -1);

    void drawTexturedRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4, const Vect3& color, int textureIndex);
    void drawTexturedCube(const Vect3& center, const Vect3& color, int textureIndex, const Mat4& rotation = Mat4::identity(), const Vect3& scale = Vect3(1.0f, 1.0f, 1.0f), const Material& material = Material());

    void drawMesh(const Mesh& mesh, const Vect3& center, const Vect3& color, const Material& material = Material(), const Mat4& rotation = Mat4::identity(), const Vect3& scale = Vect3(1.0f, 1.0f, 1.0f), int textureIndex = -1);
    void drawMesh(const Mesh& mesh, const Mat4& model, const Vect3& color, const Material& material = Material(), int textureIndex = -1);
    void drawSphere(const Vect3& center, float radius, const Vect3& color, const Material& material = Material(), int slices = 24, int stacks = 14);
    void drawSprite(const Vect3& center, const Vect3& size, int textureIndex, const Vect3& color = Vect3(1.0f, 1.0f, 1.0f), float rotation = 0.0f);
    void drawText(const Font& font, const std::string& text, const Vect3& position, float size, const Vect3& color = Vect3(1.0f, 1.0f, 1.0f));
    void drawTextScreen(const Font& font, const std::string& text, float x, float y, float sizePx, const Vect3& color = Vect3(1.0f, 1.0f, 1.0f));

    void drawPanelScreen(float x, float y, float width, float height, const Vect3& color);
    void drawPanelScreen(float x, float y, float width, float height, int textureIndex, const Vect3& color = Vect3(1.0f, 1.0f, 1.0f));

    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<uint32_t>& getIndices() const { return indices; }
    const std::vector<ModelData>& getModelData() const { return modelData; }


    uint32_t getBlendedIndexOffset() const;
    uint32_t getScreenIndexOffset() const;

private:
    uint32_t pushUniqueVertex(const Vertex& vertex);
    uint32_t addModel(const Mat4& model, const Material& material);
    void beginBlended();
    void beginScreenText();
    void emitText(const Font& font, const std::string& text, const Vect3& position, float scale, const Vect3& color);
    void drawRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4, const Vect3& color, uint32_t modelIndex, const Vect3& normal = Vect3(0.0f, 0.0f, 0.0f));
    void drawTexturedRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4, const Vect3& color, int textureIndex, uint32_t modelIndex, const Vect3& normal = Vect3(0.0f, 0.0f, 0.0f));
    void drawCubeWithTexture(const Vect3& center, const Vect3& color, int textureIndex, const Mat4& rotation, const Vect3& scale, const Material& material);
    void emitCubeModel(const Mat4& model, const Vect3& color, int textureIndex, const Material& material);
    void emitMesh(const Mesh& mesh, const Mat4& model, const Vect3& color, const Material& material, int textureIndex);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<ModelData> modelData;
    uint32_t blendedIndexStart = 0;
    bool inBlendedBatch = false;
    uint32_t screenIndexStart = 0;
    bool inScreenBatch = false;
};
