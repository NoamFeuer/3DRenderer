#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Texture.hpp"

class VulkanContext;

// A rasterized TrueType/OpenType font. Loading renders the printable ASCII
// range (32..126) into a single GPU atlas texture at a fixed pixel height;
// Renderer::drawText() then lays out glyphs from that atlas at any world size.
// Owns its atlas texture and its bindless texture index, so a Font must outlive
// the VulkanContext it was created from. Move-only.
class Font {
public:
    struct Glyph {
        // Normalized UV rect of the glyph within the atlas.
        float uvLeft = 0.0f, uvTop = 0.0f, uvRight = 0.0f, uvBottom = 0.0f;
        // Bitmap size in atlas pixels.
        float width = 0.0f, height = 0.0f;
        // Offset of the bitmap from the pen origin, in pixels.
        float bearingX = 0.0f, bearingY = 0.0f;
        // Distance to advance the pen, in pixels.
        float advance = 0.0f;
    };

    Font() = default;
    ~Font() = default;

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    Font(Font&& other) noexcept;
    Font& operator=(Font&& other) noexcept;

    // Rasterizes the font at the given pixel height into the atlas.
    static Font load(const std::string& path, int pixelHeight, VulkanContext& context);

    // Glyph for a character; non-printable/out-of-range characters map to '?'.
    const Glyph& getGlyph(char c) const;

    float getAscent() const { return ascentPx; }
    int getPixelHeight() const { return pixelHeight; }
    int getTextureIndex() const { return textureIndex; }

private:
    static constexpr int FIRST_CHAR = 32;
    static constexpr int CHAR_COUNT = 95; // 32..126, inclusive

    Texture atlas;
    std::vector<Glyph> glyphs;
    int textureIndex = -1;
    int pixelHeight = 0;
    float ascentPx = 0.0f;
};