#define STB_TRUETYPE_IMPLEMENTATION
#include "../third_party/stb_truetype.h"

#include "Font.hpp"
#include "VulkanContext.hpp"

#include <fstream>
#include <stdexcept>

Font::Font(Font&& other) noexcept
    : atlas(std::move(other.atlas)),
      glyphs(std::move(other.glyphs)),
      textureIndex(other.textureIndex),
      pixelHeight(other.pixelHeight),
      ascentPx(other.ascentPx) {
    other.textureIndex = -1;
    other.pixelHeight = 0;
    other.ascentPx = 0.0f;
}

Font& Font::operator=(Font&& other) noexcept {
    if (this != &other) {
        atlas = std::move(other.atlas);
        glyphs = std::move(other.glyphs);
        textureIndex = other.textureIndex;
        pixelHeight = other.pixelHeight;
        ascentPx = other.ascentPx;

        other.textureIndex = -1;
        other.pixelHeight = 0;
        other.ascentPx = 0.0f;
    }
    return *this;
}

const Font::Glyph& Font::getGlyph(char c) const {
    int idx = static_cast<int>(c) - FIRST_CHAR;
    if (idx < 0 || idx >= CHAR_COUNT)
        idx = '?' - FIRST_CHAR;
    return glyphs[static_cast<size_t>(idx)];
}

Font Font::load(const std::string& path, int pixelHeight, VulkanContext& context) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
        throw std::runtime_error("Failed to open font file: " + path);

    std::streamsize fileSize = file.tellg();
    std::vector<unsigned char> fontData(static_cast<size_t>(fileSize));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(fontData.data()), fileSize);
    file.close();

    // Pack the printable ASCII range into a single-channel atlas bitmap.
    constexpr int ATLAS_WIDTH = 1024;
    constexpr int ATLAS_HEIGHT = 1024;
    std::vector<unsigned char> atlasBitmap(static_cast<size_t>(ATLAS_WIDTH) * ATLAS_HEIGHT);

    stbtt_pack_context packContext;
    if (!stbtt_PackBegin(&packContext, atlasBitmap.data(), ATLAS_WIDTH, ATLAS_HEIGHT, 0, 1, nullptr))
        throw std::runtime_error("Failed to initialize font atlas packing!");

    constexpr int OVERSAMPLE = 2;
    stbtt_PackSetOversampling(&packContext, OVERSAMPLE, OVERSAMPLE);

    std::vector<stbtt_packedchar> packedChars(CHAR_COUNT);
    if (!stbtt_PackFontRange(&packContext, fontData.data(), 0,
                             static_cast<float>(pixelHeight),
                             FIRST_CHAR, CHAR_COUNT, packedChars.data())) {
        throw std::runtime_error("Failed to pack font glyphs!");
    }
    stbtt_PackEnd(&packContext);

    // Vertical metrics at the rasterized size.
    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, fontData.data(), 0))
        throw std::runtime_error("Failed to parse font file: " + path);

    int ascent = 0, descent = 0, lineGap = 0;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
    float scale = stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(pixelHeight));

    Font font;
    font.pixelHeight = pixelHeight;
    font.ascentPx = static_cast<float>(ascent) * scale;
    font.glyphs.resize(CHAR_COUNT);

    for (int i = 0; i < CHAR_COUNT; i++) {
        const stbtt_packedchar& pc = packedChars[static_cast<size_t>(i)];
        Glyph& g = font.glyphs[static_cast<size_t>(i)];
        g.uvLeft = static_cast<float>(pc.x0) / ATLAS_WIDTH;
        g.uvTop = static_cast<float>(pc.y0) / ATLAS_HEIGHT;
        g.uvRight = static_cast<float>(pc.x1) / ATLAS_WIDTH;
        g.uvBottom = static_cast<float>(pc.y1) / ATLAS_HEIGHT;
        // The packed box is in oversampled atlas pixels; xoff/yoff/xadvance are
        // in logical output pixels, so shrink the box to match.
        g.width = (static_cast<float>(pc.x1) - pc.x0) / OVERSAMPLE;
        g.height = (static_cast<float>(pc.y1) - pc.y0) / OVERSAMPLE;
        g.bearingX = pc.xoff;
        g.bearingY = pc.yoff;
        g.advance = pc.xadvance;
    }

    // Bake the alpha-coverage bitmap into a premultiplied-white RGBA atlas so
    // the blended pipeline can composite it without dark fringing.
    std::vector<uint8_t> rgba(static_cast<size_t>(ATLAS_WIDTH) * ATLAS_HEIGHT * 4);
    for (size_t i = 0; i < atlasBitmap.size(); i++) {
        uint8_t a = atlasBitmap[i];
        rgba[i * 4 + 0] = a;
        rgba[i * 4 + 1] = a;
        rgba[i * 4 + 2] = a;
        rgba[i * 4 + 3] = a;
    }

    font.atlas = Texture::create(ATLAS_WIDTH, ATLAS_HEIGHT, rgba.data(), context);
    font.textureIndex = static_cast<int>(context.registerTexture(font.atlas.getImageView()));

    return font;
}