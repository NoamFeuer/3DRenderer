#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"

#include "Texture.hpp"
#include "VulkanContext.hpp"

#include <cstring>
#include <stdexcept>

Texture::~Texture() {
    if (context == nullptr) return;

    if (imageView != VK_NULL_HANDLE) {
        vkDestroyImageView(context->getDevice(), imageView, nullptr);
        imageView = VK_NULL_HANDLE;
    }
    if (image != VK_NULL_HANDLE) {
        vkDestroyImage(context->getDevice(), image, nullptr);
        image = VK_NULL_HANDLE;
    }
    if (imageMemory != VK_NULL_HANDLE) {
        vkFreeMemory(context->getDevice(), imageMemory, nullptr);
        imageMemory = VK_NULL_HANDLE;
    }
}

Texture::Texture(Texture&& other) noexcept
    : context(other.context), image(other.image), imageMemory(other.imageMemory),
      imageView(other.imageView), width(other.width), height(other.height) {
    other.context = nullptr;
    other.image = VK_NULL_HANDLE;
    other.imageMemory = VK_NULL_HANDLE;
    other.imageView = VK_NULL_HANDLE;
    other.width = 0;
    other.height = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        this->~Texture();

        context = other.context;
        image = other.image;
        imageMemory = other.imageMemory;
        imageView = other.imageView;
        width = other.width;
        height = other.height;

        other.context = nullptr;
        other.image = VK_NULL_HANDLE;
        other.imageMemory = VK_NULL_HANDLE;
        other.imageView = VK_NULL_HANDLE;
        other.width = 0;
        other.height = 0;
    }
    return *this;
}

Texture Texture::load(const std::string& path, VulkanContext& context) {
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if (!pixels)
        throw std::runtime_error("Failed to load texture image: " + path);

    const VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    context.createBuffer(
        imageSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(context.getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(context.getDevice(), stagingBufferMemory);

    stbi_image_free(pixels);

    Texture texture;
    texture.context = &context;
    texture.width = static_cast<uint32_t>(texWidth);
    texture.height = static_cast<uint32_t>(texHeight);

    constexpr VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    context.createImage(
        texture.width, texture.height, format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        texture.image, texture.imageMemory);

    context.transitionImageLayout(
        texture.image, format,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    context.copyBufferToImage(stagingBuffer, texture.image, texture.width, texture.height);
    context.transitionImageLayout(
        texture.image, format,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    texture.imageView = context.createImageView(texture.image, format, VK_IMAGE_ASPECT_COLOR_BIT);

    vkDestroyBuffer(context.getDevice(), stagingBuffer, nullptr);
    vkFreeMemory(context.getDevice(), stagingBufferMemory, nullptr);

    return texture;
}