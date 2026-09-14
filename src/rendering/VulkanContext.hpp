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

    // Registers a texture image view in the bindless descriptor set and
    // returns the index to attach to vertices. The image view must stay
    // alive (and valid) for the lifetime of the context.
    uint32_t registerTexture(VkImageView imageView);

    // Resource helpers shared with Texture and other resource classes.
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void createImage(
        uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory,
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createBuffer(
        VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);
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

    // Copies the given vertices into the GPU-visible vertex buffer.
    // Call this once per frame, before drawFrame(), with whatever the
    // Renderer accumulated that frame.
    void updateVertexBuffer(const std::vector<Vertex>& vertices);

    // Copies the given indices into the GPU-visible index buffer.
    // Call this once per frame, before drawFrame(), with whatever the
    // Renderer accumulated that frame.
    void updateIndexBuffer(const std::vector<uint32_t>& indices);

    // Copies the per-object model data (indexed by each vertex's modelIndex)
    // into the GPU-visible storage buffer. Call once per frame, before
    // drawFrame(), with whatever the Renderer accumulated that frame.
    void updateModelMatrixBuffer(const std::vector<ModelData>& modelData);

    // Tells the context which index splits opaque (drawn with the opaque
    // pipeline) from blended (sprites/text, drawn with the blended pipeline)
    // content. Pass Renderer::getBlendedIndexOffset() each frame.
    void setBlendedIndexOffset(uint32_t offset) { blendedIndexOffset = offset; }
    void setScreenIndexOffset(uint32_t offset) { screenIndexOffset = offset; }

    // Sets the matrix pushed to the vertex shader each draw. Call once per
    // frame, before drawFrame(), with (projection * view) for the frame's camera.
    void setViewProjection(const Mat4& viewProjection);

    // Uploads the frame's lighting state (camera position, ambient, lights) to
    // the lighting UBO. Call once per frame before drawFrame().
    void setLighting(const Vect3& cameraPosition, const LightingState& state);

    // Configures the directional shadow map for the frame. `lightViewProj`
    // maps world space into the light's orthographic clip space; `lightIndex`
    // is the index of the shadow-casting light in the frame's light list, or -1
    // to disable shadows. Call once per frame before drawFrame().
    void setShadowLight(const Mat4& lightViewProj, int lightIndex);

    // Configures the procedural sky. Enabled by default; set the two gradient
    // colors and the centre (usually the camera position) each frame.
    void setSkyEnabled(bool enabled) { skyEnabled = enabled; }
    void setSkyColor(const Vect3& top, const Vect3& bottom) {
        skyTopColor = top;
        skyBottomColor = bottom;
    }
    void setSkyCenter(const Vect3& center) { skyCenter = center; }

    void drawFrame(Window& window);
    void waitIdle();

    // Debug hook: on every subsequent drawFrame, copies the rendered
    // (post-resolve) swapchain image into a host-visible buffer so the frame
    // can be inspected off-screen. Pixels are tightly packed W*H*4 bytes in
    // swapchain-format channel order (RGBA or BGRA). Do not call alongside
    // normal use beyond debugging.
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

    // Multisample count for the color/depth attachments (1 = no MSAA).
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    // Multisampled color image that the scene is rendered into, then resolved
    // to the (single-sample) swapchain image. One image shared across all
    // swapchain framebuffers, sized with the swapchain.
    VkImage colorImage = VK_NULL_HANDLE;
    VkDeviceMemory colorImageMemory = VK_NULL_HANDLE;
    VkImageView colorImageView = VK_NULL_HANDLE;

    // Depth buffer — one image, shared across all swapchain framebuffers.
    // Sized to match the swapchain, so it's torn down/recreated alongside it.
    VkImage depthImage = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory = VK_NULL_HANDLE;
    VkImageView depthImageView = VK_NULL_HANDLE;
    VkFormat depthFormat = VK_FORMAT_UNDEFINED;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline graphicsPipeline = VK_NULL_HANDLE;
    // Alpha-blended pipeline used for sprites/text (premultiplied alpha,
    // depth-test on but not writing). Draws the second half of the index range.
    VkPipeline blendedPipeline = VK_NULL_HANDLE;
    // Sky pipeline: depth writes off, renders the gradient backdrop cube.
    VkPipeline skyPipeline = VK_NULL_HANDLE;

    // Index offset within the frame's index buffer where blended content begins.
    // Content before it is drawn with `graphicsPipeline`, from it onwards with
    // `blendedPipeline`. UINT32_MAX means the whole frame is opaque.
    uint32_t blendedIndexOffset = std::numeric_limits<uint32_t>::max();

    // Index offset where screen-space (HUD) blended content begins.
    // UINT32_MAX means no screen-space content this frame.
    uint32_t screenIndexOffset = std::numeric_limits<uint32_t>::max();

    // Bindless texture support — one combined-image-sampler array covering
    // every registered texture, bound once per frame in recordCommandBuffer().
    static constexpr uint32_t MAX_TEXTURES = 64;
    VkDescriptorSetLayout textureDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkDescriptorSet textureDescriptorSet = VK_NULL_HANDLE;
    VkSampler textureSampler = VK_NULL_HANDLE;
    std::vector<VkImageView> textureImageViews;

    // Per-object model matrices in a storage buffer, indexed by each vertex's
    // modelIndex attribute. Must match Renderer::MAX_MODELS.
    static constexpr uint32_t MAX_MODELS = 4096;
    VkDescriptorSetLayout modelDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet modelDescriptorSet = VK_NULL_HANDLE;
    VkBuffer modelBuffer = VK_NULL_HANDLE;
    VkDeviceMemory modelBufferMemory = VK_NULL_HANDLE;
    void* modelBufferMapped = nullptr;
    uint32_t modelCount = 0;

    // Per-frame lighting uniform buffer (set 2).
    VkDescriptorSetLayout lightingDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet lightingDescriptorSet = VK_NULL_HANDLE;
    VkBuffer lightingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory lightingBufferMemory = VK_NULL_HANDLE;
    void* lightingBufferMapped = nullptr;

    // Procedural sky state. The sky cube is re-centred on `skyCenter` (the
    // camera) every frame before upload.
    bool skyEnabled = true;
    Vect3 skyTopColor{ 0.18f, 0.26f, 0.45f };
    Vect3 skyBottomColor{ 0.72f, 0.80f, 0.85f };
    Vect3 skyCenter{ 0.0f, 0.0f, 0.0f };

    // Directional shadow mapping. The engine computes a tight orthographic
    // view-projection for the first directional light each frame and the whole
    // scene is rendered into a depth-only shadow map before the main color
    // pass; triangle.frag samples it for the shadow-casting light.
    static constexpr uint32_t SHADOW_MAP_SIZE = 2048;
    VkImage shadowImage = VK_NULL_HANDLE;
    VkDeviceMemory shadowImageMemory = VK_NULL_HANDLE;
    VkImageView shadowImageView = VK_NULL_HANDLE;
    VkSampler shadowSampler = VK_NULL_HANDLE;
    VkRenderPass shadowRenderPass = VK_NULL_HANDLE;
    VkFramebuffer shadowFramebuffer = VK_NULL_HANDLE;
    VkPipeline shadowPipeline = VK_NULL_HANDLE;
    // Separate pipeline layout for the shadow pass: only the frame matrix push
    // constant and the model storage buffer set are used, no fragment stage.
    VkPipelineLayout shadowPipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout shadowDescriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet shadowDescriptorSet = VK_NULL_HANDLE;
    // Light-space orthographic matrix uploaded to the lighting UBO so the
    // fragment shader can transform world positions into the shadow map.
    Mat4 shadowLightMatrix;
    // Index (into the light list) of the light whose shadow map is active,
    // or -1 when no shadow-casting light is configured.
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

    // Matrix pushed to the vertex shader each frame via setViewProjection().
    Mat4 viewProjectionMatrix;

    // Vertex buffer is allocated once, with fixed capacity, and persistently
    // mapped — every frame we just memcpy new data into it rather than
    // recreating buffers/memory each time.
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

    // Debug frame readback (see enableDebugReadback).
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
