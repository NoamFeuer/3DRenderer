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

// Screen-space (HUD) quads are emitted at this Z which must match the near
// plane used by VulkanContext's orthographic screen projection.
inline constexpr float HUD_Z = -0.1f;

class Renderer {
public:
    // Must match the storage buffer capacity in VulcanContext.
    static constexpr uint32_t MAX_MODELS = 4096;

    // Defaults to a single soft directional light so scenes render with shape
    // immediately; replace `lighting` with your own lights each frame.
    LightingState lighting;

    Renderer();

    void beginFrame();

    // Primitive shapes. Geometry drawn without a model matrix (raw triangles,
    // unpainted rectangles/circles) renders unlit: the color is used as-is.
    void drawTriangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& color);
    void drawRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4, const Vect3& color);

    void drawCircle(const Vect3& center, float radius, const Vect3& color, int segments = 32);

    // Lit cube. `material` (the 5th/6th argument) is optional and defaults to
    // a mild plastic look; pass it last when customizing, e.g.
    //   renderer.drawCube(center, color, Mat4::identity(), Vect3(1), Material{...});
    void drawCube(const Vect3& center, const Vect3& color,
                  const Mat4& rotation = Mat4::identity(),
                  const Vect3& scale = Vect3(1.0f, 1.0f, 1.0f),
                  const Material& material = Material());

    // Textured variants. `textureIndex` comes from Engine::loadTexture();
    // pass -1 to sample no texture (color used as-is).
    void drawTexturedRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4,
                          const Vect3& color, int textureIndex);
    void drawTexturedCube(const Vect3& center, const Vect3& color, int textureIndex,
                      const Mat4& rotation = Mat4::identity(),
                      const Vect3& scale = Vect3(1.0f, 1.0f, 1.0f),
                      const Material& material = Material());

    // A full mesh (vertices + indices) — see Mesh.hpp. Transforms and material
    // work like drawCube; textureIndex = -1 uses the vertex color alone.
    void drawMesh(const Mesh& mesh, const Vect3& center, const Vect3& color,
                  const Material& material = Material(),
                  const Mat4& rotation = Mat4::identity(),
                  const Vect3& scale = Vect3(1.0f, 1.0f, 1.0f),
                  int textureIndex = -1);

    // UV-sphere built on the fly, convenient for sims and debugging.
    void drawSphere(const Vect3& center, float radius, const Vect3& color,
                    const Material& material = Material(),
                    int slices = 24, int stacks = 14);

    // 2D sprite: a textured quad in the XY plane centred on `center` with the
    // given size (world units). `rotation` is a Z-axis rotation in radians.
    // Draw sprites with the Camera in Orthographic mode, placing them in front
    // of the camera (e.g. camera at z=+5 and sprites at z=0). Sprites are drawn
    // with alpha blending and never lit.
    void drawSprite(const Vect3& center, const Vect3& size, int textureIndex,
                    const Vect3& color = Vect3(1.0f, 1.0f, 1.0f), float rotation = 0.0f);

    // Draws a text string in world space. `position` is the top-left of the
    // text, `size` its height in world units, `color` its tint. Text is alpha
    // blended and never lit.
    void drawText(const Font& font, const std::string& text, const Vect3& position, float size,
                  const Vect3& color = Vect3(1.0f, 1.0f, 1.0f));

    // Draws a text string in screen space (HUD). `(x, y)` is the top-left of
    // the text in pixels from the window corner, `sizePx` is the desired pixel
    // height, `color` its tint.  The string is drawn on top of all world
    // geometry regardless of camera position.
    void drawTextScreen(const Font& font, const std::string& text, float x, float y,
                        float sizePx,
                        const Vect3& color = Vect3(1.0f, 1.0f, 1.0f));

    // Draws a solid-color quad in screen space (HUD panel). `(x, y)` is the
    // top-left in pixels, `width`/`height` its size. Drawn in the same screen
    // pass as drawTextScreen, so it stays fixed regardless of the camera.
    // Draw panels before any text you want on top of them.
    void drawPanelScreen(float x, float y, float width, float height, const Vect3& color);

    // Textured HUD panel using a texture index from Engine::loadTexture();
    // `color` tints it (white = as-is). Pass -1 for a solid-color panel.
    void drawPanelScreen(float x, float y, float width, float height, int textureIndex,
                         const Vect3& color = Vect3(1.0f, 1.0f, 1.0f));

    const std::vector<Vertex>& getVertices() const { return vertices; }
    const std::vector<uint32_t>& getIndices() const { return indices; }
    const std::vector<ModelData>& getModelData() const { return modelData; }

    // Index offset that splits opaque (drawn first) from blended (drawn last)
    // content within the index buffer. UINT32_MAX means no blended content.
    uint32_t getBlendedIndexOffset() const;

    // Index offset that splits world blended content from screen-space blended
    // content. UINT32_MAX means no screen content.
    uint32_t getScreenIndexOffset() const;

private:
    uint32_t pushUniqueVertex(const Vertex& vertex);
    uint32_t addModel(const Mat4& model, const Material& material);
    void beginBlended();
    void beginScreenText();
    void emitText(const Font& font, const std::string& text, const Vect3& position,
                  float scale, const Vect3& color);
    void drawRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4,
                       const Vect3& color, uint32_t modelIndex,
                       const Vect3& normal = Vect3(0.0f, 0.0f, 0.0f));
    void drawTexturedRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4,
                               const Vect3& color, int textureIndex, uint32_t modelIndex,
                               const Vect3& normal = Vect3(0.0f, 0.0f, 0.0f));
    void drawCubeWithTexture(const Vect3& center, const Vect3& color, int textureIndex,
                         const Mat4& rotation, const Vect3& scale, const Material& material);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<ModelData> modelData;
    uint32_t blendedIndexStart = 0;
    bool inBlendedBatch = false;
    uint32_t screenIndexStart = 0;
    bool inScreenBatch = false;
};