#pragma once

#include <cstdint>

#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"
#include "Material.hpp"
#include "Light.hpp"

struct alignas(16) ModelData {
    Mat4 model;
    Mat4 normalMatrix;
    Vect3 emission;
    float lit;
    Vect3 specular;
    float shininess;
};
static_assert(sizeof(ModelData) == 160, "ModelData must match the GLSL struct (4 x vec4 aligned)");

struct alignas(16) GpuLight {
    Vect3 position;
    float type;          // Light::Type stored as float (GLSL reads vec4.a as float)
    Vect3 direction;
    float spotOuterCos;
    Vect3 color;
    float intensity;
    Vect3 attenuation;
    float spotInnerCos;
};
static_assert(sizeof(GpuLight) == 64, "GpuLight must be 4 x vec4");

inline constexpr uint32_t MAX_LIGHTS = 16;

struct alignas(16) LightingData {
    Vect3 cameraPosition;
    float pad0;
    Vect3 ambientColor;
    float pad1;
    float lightCount;    // stored as float (GLSL reads vec4.x as float)
    float pad2a;
    float pad2b;
    float pad2c;
    GpuLight lights[MAX_LIGHTS];
};
static_assert(sizeof(LightingData) == 48 + sizeof(GpuLight) * MAX_LIGHTS, "LightingData layout");

inline GpuLight toGpuLight(const Light& light) {
    GpuLight g{};
    g.position = light.position;
    g.type = static_cast<float>(static_cast<uint32_t>(light.type));
    g.direction = light.direction;
    g.spotOuterCos = light.spotOuterCos;
    g.color = light.color;
    g.intensity = light.intensity;
    g.attenuation = light.attenuation;
    g.spotInnerCos = light.spotInnerCos;
    return g;
}