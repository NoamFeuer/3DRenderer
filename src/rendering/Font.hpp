#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Texture.hpp"

class VulkanContext;

class Font {
public:
    struct Glyph {
        float uvLeft = 0.0f, uvTop = 0.0f, uvRight = 0.0f, uvBottom = 0.0f;
        float width = 0.0f, height = 0.0f;
        float bearingX = 0.0f, bearingY = 0.0f;
        float advance = 0.0f;
    };

    Font() = default;
    ~Font() = default;

    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;

    Font(Font&& other) noexcept;
    Font& operator=(Font&& other) noexcept;

    static Font load(const std::string& path, int pixelHeight, VulkanContext& context);

    const Glyph& getGlyph(char c) const;

    float getAscent() const { return ascentPx; }
    int getPixelHeight() const { return pixelHeight; }
    int getTextureIndex() const { return textureIndex; }

private:
    static constexpr int FIRST_CHAR = 32;
    static constexpr int CHAR_COUNT = 95;

    Texture atlas;
    std::vector<Glyph> glyphs;
    int textureIndex = -1;
    int pixelHeight = 0;
    float ascentPx = 0.0f;
};
