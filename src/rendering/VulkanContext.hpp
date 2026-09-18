#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <limits>

#include "../core/Window.hpp"
#include "Vertex.hpp"
#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"
#include "GpuData.hpp"

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanContext {
public:
    void init(Window& window);
    void cleanup();

    uint32_t registerTexture(VkImageView imageView);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkInstance getInstance() const { return instance; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    VkDevice getDevice() const { return device; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkQueue getPresentQueue() const { return presentQueue; }
    VkSurfaceKHR getSurface() const { return surface; }
    VkSwapchainKHR getSwapChain() const { return swapChain; }
    VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
    VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
    const std::vector<VkImageView>& getSwapChainImageViews() const { return swapChainImageViews; }

    void updateVertexBuffer(const std::vector<Vertex>& vertices);
    void updateIndexBuffer(const std::vector<uint32_t>& indices);
    void updateModelMatrixBuffer(const std::vector<ModelData>& modelData);

    void setBlendedIndexOffset(uint32_t offset) { blendedIndexOffset = offset; }
    void setScreenIndexOffset(uint32_t offset) { screenIndexOffset = offset; }
    void setViewProjection(const Mat4& viewProjection);
    void setLighting(const Vect3& cameraPosition, const LightingState& state);
    void setShadowLight(const Mat4& lightViewProj, int lightIndex);
    void setSkyEnabled(bool enabled) { skyEnabled = enabled; }
    void setSkyColor(const Vect3& top, const Vect3& bottom) {
        skyTopColor = top;
        skyBottomColor = bottom;
    }
    void setSkyCenter(const Vect3& center) { skyCenter = center; }

    void drawFrame(Window& window);
    void waitIdle();

    void enableDebugReadback();
    const unsigned char* debugReadbackPixels() const { return debugReadbackPixelsPtr; }
    uint32_t debugReadbackWidth() const { return debugReadbackExtent.width; }
    uint32_t debugReadbackHeight() const { return debugReadbackExtent.height; }

private:
    VkInstance instance = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapChainExtent{};
    std::vector<VkImageView> swapChainImageViews;

    VkRenderPass renderPass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    VkImage colorImage = VK_NULL_HANDLE;
    VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
    VkImageView colorImageView = VK_NULL_HANDLE;

    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    VkPipeline blendedPipeline = VK_NULL_HANDLE;
    VkPipeline skyPipeline = VK_NULL_HANDLE;

    uint32_t blendedIndexOffset = std::numeric_limits<uint32_t>::max();
    uint32_t screenIndexOffset = std::numeric_limits<uint32_t>::max();

    static constexpr uint32_t MAX_TEXTURES = 64;
    VkDescriptorSetLayout textureDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet textureDescriptorSet = VK_NULL_HANDLE;
    VkSampler textureSampler = VK_NULL_HANDLE;
    std::vector<VkImageView> textureImageViews;

    static constexpr uint32_t MAX_MODELS = 4096;
    VkDescriptorSetLayout modelDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet modelDescriptorSet = VK_NULL_HANDLE;
    VkBuffer modelBuffer = VK_NULL_HANDLE;
    VkDeviceMemory modelBufferMemory = VK_NULL_HANDLE;
    void* modelBufferMapped = nullptr;
    uint32_t modelCount = 0;

    VkDescriptorSetLayout lightingDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet lightingDescriptorSet = VK_NULL_HANDLE;
    VkBuffer lightingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory lightingBufferMemory = VK_NULL_HANDLE;
    void* lightingBufferMapped = nullptr;

    bool skyEnabled = true;
    Vect3 skyTopColor{ 0.18f, 0.26f, 0.45f };
    Vect3 skyBottomColor{ 0.72f, 0.80f, 0.85f };
    Vect3 skyCenter{ 0.0f, 0.0f, 0.0f };

    static constexpr uint32_t SHADOW_MAP_SIZE = 2048;
    VkImage shadowImage = VK_NULL_HANDLE;
    VkDeviceMemory shadowImageMemory = VK_NULL_HANDLE;
    VkImageView shadowImageView = VK_NULL_HANDLE;
    VkSampler shadowSampler = VK_NULL_HANDLE;
    VkRenderPass shadowRenderPass = VK_NULL_HANDLE;
    VkFramebuffer shadowFramebuffer = VK_NULL_HANDLE;
    VkPipeline shadowPipeline = VK_NULL_HANDLE;
    VkPipelineLayout shadowPipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout shadowDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet shadowDescriptorSet = VK_NULL_HANDLE;
    Mat4 shadowLightMatrix;
    int shadowLightIndex = -1;

    void createShadowDescriptorSetLayout();
    void allocateShadowDescriptorSet();
    void updateShadowDescriptor();
    void createShadowResources();
    void createShadowRenderPass();
    void createShadowPipeline();
    void createShadowSampler();

    void createTextureSampler();
    void createTextureDescriptorSetLayout();
    void createDescriptorPool();
    void allocateTextureDescriptorSet();
    void updateTextureDescriptors();

    void createModelDescriptorSetLayout();
    void allocateModelDescriptorSet();
    void updateModelDescriptor();
    void createModelBuffer();

    void createLightingDescriptorSetLayout();
    void allocateLightingDescriptorSet();
    void updateLightingDescriptor();
    void createLightingBuffer();

    Mat4 viewProjectionMatrix;

    static constexpr uint32_t MAX_VERTICES = 100000;
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    void* vertexBufferMapped = nullptr;
    uint32_t vertexCount = 0;

    static constexpr uint32_t MAX_INDICES = 100000;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    void* indexBufferMapped = nullptr;
    uint32_t indexCount = 0;

    VkCommandPool commandPool = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence inFlightFence = VK_NULL_HANDLE;

    bool debugReadbackEnabled = false;
    VkExtent2D debugReadbackExtent{};
    VkBuffer debugReadbackBuffer = VK_NULL_HANDLE;
    VkDeviceMemory debugReadbackMemory = VK_NULL_HANDLE;
    void* debugReadbackMapped = nullptr;
    const unsigned char* debugReadbackPixelsPtr = nullptr;
    void createDebugReadbackBuffer();

    void createInstance();
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();

    void createSurface(Window& window);

    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    void createLogicalDevice();

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, Window& window);
    void createSwapChain(Window& window);
    void createImageViews();
    void cleanupSwapChain();
    void recreateSwapChain(Window& window);

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                  VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();
    void createDepthResources();
    void createColorResources();

    void createRenderPass();
    void createGraphicsPipeline();
    VkPipeline buildPipeline(const char* vertexShader, const char* fragmentShader,
                             const VkPipelineColorBlendAttachmentState& colorBlendAttachment,
                             VkBool32 depthWriteEnable);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    void createFramebuffers();
    VkSampleCountFlagBits chooseSampleCount();

    void createCommandPool();
    void createCommandBuffer();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void createSyncObjects();

    void createVertexBuffer();
    void createIndexBuffer();

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

#ifdef NDEBUG
    static constexpr bool enableValidationLayers = false;
#else
    static constexpr bool enableValidationLayers = true;
#endif

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
};
