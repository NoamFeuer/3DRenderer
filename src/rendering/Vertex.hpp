#pragma once

#include <vulkan/vulkan.h>
#include <array>

#include "../math/Vect3.hpp"

// A single vertex's data, as it will live in a GPU buffer.
// If you add fields here (normal, uv, etc.), you MUST also update
// getAttributeDescriptions() below, and the matching `layout(location = N) in`
// declarations in the vertex shader — the three have to agree exactly.
struct Vertex {
    Vect3 position;
    Vect3 color;

    // Describes how one Vertex's worth of bytes are laid out in the buffer —
    // stride (size of one vertex) and whether to advance per-vertex or per-instance.
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    // Describes each individual field within a Vertex: which shader "location"
    // it maps to, its format (VK_FORMAT_R32G32B32_SFLOAT = 3 floats), and its
    // byte offset within the struct.
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0; // matches `layout(location = 0)` in the vertex shader
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1; // matches `layout(location = 1)`
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};
