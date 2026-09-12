#include "Renderer.hpp"
#include "Font.hpp"
#include <cmath>
#include <limits>
#include <stdexcept>

void Renderer::beginFrame() {
    vertices.clear();
    indices.clear();
    modelMatrices.clear();
    blendedIndexStart = 0;
    inBlendedBatch = false;
    screenIndexStart = 0;
    inScreenBatch = false;
}

uint32_t Renderer::getBlendedIndexOffset() const {
    return inBlendedBatch ? blendedIndexStart : std::numeric_limits<uint32_t>::max();
}

uint32_t Renderer::getScreenIndexOffset() const {
    return inScreenBatch ? screenIndexStart : std::numeric_limits<uint32_t>::max();
}

void Renderer::beginBlended() {
    if (!inBlendedBatch) {
        inBlendedBatch = true;
        blendedIndexStart = static_cast<uint32_t>(indices.size());
    }
}

void Renderer::beginScreenText() {
    if (!inScreenBatch) {
        inScreenBatch = true;
        screenIndexStart = static_cast<uint32_t>(indices.size());
    }
}

uint32_t Renderer::addModel(const Mat4& model) {
    if (modelMatrices.size() >= MAX_MODELS)
        throw std::runtime_error("Too many transformed objects for the model matrix buffer! Increase MAX_MODELS.");
    modelMatrices.push_back(model);
    return static_cast<uint32_t>(modelMatrices.size() - 1);
}

uint32_t Renderer::pushUniqueVertex(const Vertex& vertex) {
    for (size_t i = 0; i < vertices.size(); i++) {
        const Vertex& v = vertices[i];
        if (v.position.x == vertex.position.x &&
            v.position.y == vertex.position.y &&
            v.position.z == vertex.position.z &&
            v.color.x == vertex.color.x &&
            v.color.y == vertex.color.y &&
            v.color.z == vertex.color.z &&
            v.u == vertex.u &&
            v.v == vertex.v &&
            v.textureIndex == vertex.textureIndex &&
            v.modelIndex == vertex.modelIndex) {
            return static_cast<uint32_t>(i);
        }
    }
    vertices.push_back(vertex);
    return static_cast<uint32_t>(vertices.size() - 1);
}

void Renderer::drawTriangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& color) {
    indices.push_back(pushUniqueVertex({ p1, color }));
    indices.push_back(pushUniqueVertex({ p2, color }));
    indices.push_back(pushUniqueVertex({ p3, color }));
}

void Renderer::drawRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4, const Vect3& color) {
    drawRectangle(p1, p2, p3, p4, color, -1);
}

void Renderer::drawRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4,
                             const Vect3& color, uint32_t modelIndex) {
    int idx = static_cast<int>(modelIndex);
    indices.push_back(pushUniqueVertex({ p1, color, 0.0f, 0.0f, -1, idx }));
    indices.push_back(pushUniqueVertex({ p2, color, 0.0f, 0.0f, -1, idx }));
    indices.push_back(pushUniqueVertex({ p3, color, 0.0f, 0.0f, -1, idx }));
    indices.push_back(pushUniqueVertex({ p1, color, 0.0f, 0.0f, -1, idx }));
    indices.push_back(pushUniqueVertex({ p3, color, 0.0f, 0.0f, -1, idx }));
    indices.push_back(pushUniqueVertex({ p4, color, 0.0f, 0.0f, -1, idx }));
}

void Renderer::drawTexturedRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4,
                                     const Vect3& color, int textureIndex) {
    drawTexturedRectangle(p1, p2, p3, p4, color, textureIndex, -1);
}

void Renderer::drawTexturedRectangle(const Vect3& p1, const Vect3& p2, const Vect3& p3, const Vect3& p4,
                                     const Vect3& color, int textureIndex, uint32_t modelIndex) {
    int idx = static_cast<int>(modelIndex);
    Vertex v0{ p1, color, 0.0f, 0.0f, textureIndex, idx };
    Vertex v1{ p2, color, 1.0f, 0.0f, textureIndex, idx };
    Vertex v2{ p3, color, 1.0f, 1.0f, textureIndex, idx };
    Vertex v3{ p4, color, 0.0f, 1.0f, textureIndex, idx };

    indices.push_back(pushUniqueVertex(v0));
    indices.push_back(pushUniqueVertex(v1));
    indices.push_back(pushUniqueVertex(v2));
    indices.push_back(pushUniqueVertex(v0));
    indices.push_back(pushUniqueVertex(v2));
    indices.push_back(pushUniqueVertex(v3));
}

void Renderer::drawCircle(const Vect3& center, float radius, const Vect3& color, int segments) {
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

void Renderer::drawSprite(const Vect3& center, const Vect3& size, int textureIndex,
                          const Vect3& color, float rotation) {
    beginBlended();

    Mat4 model = Mat4::translation(center.x, center.y, center.z) *
                 Mat4::rotationZ(rotation) *
                 Mat4::scale(size.x, size.y, size.z);
    uint32_t modelIndex = addModel(model);

    Vertex v0{ Vect3(-0.5f, -0.5f, 0.0f), color, 0.0f, 0.0f, textureIndex, static_cast<int>(modelIndex) };
    Vertex v1{ Vect3( 0.5f, -0.5f, 0.0f), color, 1.0f, 0.0f, textureIndex, static_cast<int>(modelIndex) };
    Vertex v2{ Vect3( 0.5f,  0.5f, 0.0f), color, 1.0f, 1.0f, textureIndex, static_cast<int>(modelIndex) };
    Vertex v3{ Vect3(-0.5f,  0.5f, 0.0f), color, 0.0f, 1.0f, textureIndex, static_cast<int>(modelIndex) };

    indices.push_back(pushUniqueVertex(v0));
    indices.push_back(pushUniqueVertex(v1));
    indices.push_back(pushUniqueVertex(v2));
    indices.push_back(pushUniqueVertex(v0));
    indices.push_back(pushUniqueVertex(v2));
    indices.push_back(pushUniqueVertex(v3));
}

void Renderer::drawText(const Font& font, const std::string& text, const Vect3& position, float size,
                        const Vect3& color) {
    if (text.empty())
        return;

    beginBlended();
    emitText(font, text, position, size / static_cast<float>(font.getPixelHeight()), color);
}

void Renderer::drawTextScreen(const Font& font, const std::string& text, float x, float y,
                              float sizePx, const Vect3& color) {
    if (text.empty())
        return;

    beginScreenText();
    emitText(font, text, Vect3(x, y, HUD_Z), sizePx / static_cast<float>(font.getPixelHeight()), color);
}

void Renderer::emitText(const Font& font, const std::string& text, const Vect3& position,
                        float scale, const Vect3& color) {
    const int textureIndex = font.getTextureIndex();
    const float baselineY = position.y + font.getAscent() * scale;
    float penX = 0.0f;

    for (char c : text) {
        if (c < ' ' || c > '~') {
            penX += font.getGlyph(' ').advance;
            continue;
        }

        const Font::Glyph& g = font.getGlyph(c);

        if (g.width > 0.0f && g.height > 0.0f) {
            float x0 = position.x + (penX + g.bearingX) * scale;
            float y0 = baselineY + g.bearingY * scale;
            float x1 = x0 + g.width * scale;
            float y1 = y0 + g.height * scale;

            Vertex v0{ Vect3(x0, y0, position.z), color, g.uvLeft, g.uvTop, textureIndex, -1 };
            Vertex v1{ Vect3(x1, y0, position.z), color, g.uvRight, g.uvTop, textureIndex, -1 };
            Vertex v2{ Vect3(x1, y1, position.z), color, g.uvRight, g.uvBottom, textureIndex, -1 };
            Vertex v3{ Vect3(x0, y1, position.z), color, g.uvLeft, g.uvBottom, textureIndex, -1 };

            indices.push_back(pushUniqueVertex(v0));
            indices.push_back(pushUniqueVertex(v1));
            indices.push_back(pushUniqueVertex(v2));
            indices.push_back(pushUniqueVertex(v0));
            indices.push_back(pushUniqueVertex(v2));
            indices.push_back(pushUniqueVertex(v3));
        }

        penX += g.advance;
    }
}

void Renderer::drawCube(const Vect3& center, const Vect3& color, const Mat4& rotation, const Vect3& scale) {
    drawCubeWithTexture(center, color, -1, rotation, scale);
}

void Renderer::drawTexturedCube(const Vect3& center, const Vect3& color, int textureIndex,
                                const Mat4& rotation, const Vect3& scale) {
    drawCubeWithTexture(center, color, textureIndex, rotation, scale);
}

void Renderer::drawCubeWithTexture(const Vect3& center, const Vect3& color, int textureIndex,
                                   const Mat4& rotation, const Vect3& scale) {
    // The rotation/scale/translation are baked into a model matrix and applied
    // on the GPU; only local-space (-0.5..0.5) corners are pushed to the vertex
    // buffer.
    Mat4 model = Mat4::translation(center.x, center.y, center.z) * rotation * Mat4::scale(scale.x, scale.y, scale.z);
    uint32_t modelIndex = addModel(model);

    float h = 0.5f;

    std::vector<Vect3> localCorners = {
        Vect3(-h, -h, -h), Vect3( h, -h, -h), Vect3( h,  h, -h), Vect3(-h,  h, -h),
        Vect3(-h, -h,  h), Vect3( h, -h,  h), Vect3( h,  h,  h), Vect3(-h,  h,  h)
    };

    Vect3 p0 = localCorners[0];
    Vect3 p1 = localCorners[1];
    Vect3 p2 = localCorners[2];
    Vect3 p3 = localCorners[3];
    Vect3 p4 = localCorners[4];
    Vect3 p5 = localCorners[5];
    Vect3 p6 = localCorners[6];
    Vect3 p7 = localCorners[7];

    if (textureIndex >= 0) {
        drawTexturedRectangle(p0, p1, p2, p3, color, textureIndex, modelIndex);
        drawTexturedRectangle(p5, p4, p7, p6, color, textureIndex, modelIndex);
        drawTexturedRectangle(p4, p0, p3, p7, color, textureIndex, modelIndex);
        drawTexturedRectangle(p1, p5, p6, p2, color, textureIndex, modelIndex);
        drawTexturedRectangle(p3, p2, p6, p7, color, textureIndex, modelIndex);
        drawTexturedRectangle(p4, p5, p1, p0, color, textureIndex, modelIndex);
    }
    else {
        drawRectangle(p0, p1, p2, p3, color, modelIndex);
        drawRectangle(p5, p4, p7, p6, color, modelIndex);
        drawRectangle(p4, p0, p3, p7, color, modelIndex);
        drawRectangle(p1, p5, p6, p2, color, modelIndex);
        drawRectangle(p3, p2, p6, p7, color, modelIndex);
        drawRectangle(p4, p5, p1, p0, color, modelIndex);
    }
}
