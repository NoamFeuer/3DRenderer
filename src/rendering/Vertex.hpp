#pragma once

#include <vulkan/vulkan.h>
#include <array>

#include "../math/Vect3.hpp"

struct Vertex {
    Vect3 position;
    Vect3 color;
    float u = 0.0f;
    float v = 0.0f;
    int textureIndex = -1;
    int modelIndex = -1;
    Vect3 normal{ 0.0f, 0.0f, 0.0f };

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    // Binding 1: one int per instance holding the modelData index. Carries the
    // per-draw transform for GPU-resident static meshes, whose vertices store
    // modelIndex = -1. The dynamic (streamed) path sets this to -1 and reads the
    // vertex attribute instead.
    static VkVertexInputBindingDescription getInstanceBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 1;
        bindingDescription.stride = sizeof(int32_t);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        return bindingDescription;
    }

    static std::array<VkVertexInputBindingDescription, 2> getBindingDescriptions() {
        return { getBindingDescription(), getInstanceBindingDescription() };
    }

    static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, u);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[3].offset = offsetof(Vertex, textureIndex);

        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[4].offset = offsetof(Vertex, modelIndex);

        attributeDescriptions[5].binding = 0;
        attributeDescriptions[5].location = 5;
        attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[5].offset = offsetof(Vertex, normal);

        attributeDescriptions[6].binding = 1;
        attributeDescriptions[6].location = 6;
        attributeDescriptions[6].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[6].offset = 0;

        return attributeDescriptions;
    }
};