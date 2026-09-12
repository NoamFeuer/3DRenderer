#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <string>

class VulkanContext;

// A single 2D texture loaded from disk (PNG/JPG/etc. via stb_image) into a
// GPU-visible image. Register its image view with VulkanContext::registerTexture()
// to get a bindless index usable by draw calls.
//
// A Texture must outlive the VulkanContext it was created from, and any image
// view it provides must remain valid while the context references it.
class Texture {
public:
    Texture() = default;
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    static Texture load(const std::string& path, VulkanContext& context);

    // Creates a GPU texture from raw R8G8B8A8 pixels in memory (used e.g. for
    // procedurally generated images like font atlases).
    static Texture create(uint32_t width, uint32_t height, const uint8_t* rgbaPixels, VulkanContext& context);

    VkImageView getImageView() const { return imageView; }
    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }

private:
    static void uploadToGpu(VulkanContext& context, Texture& texture,
                            uint32_t width, uint32_t height, const uint8_t* rgbaPixels);
    VulkanContext* context = nullptr;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    uint32_t width = 0;
    uint32_t height = 0;
};