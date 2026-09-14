#version 450

layout(push_constant, row_major) uniform PushConstants {
    mat4 viewProjection;
} pc;

// Per-object transform + material. Matches the CPU-side ModelData struct in
// src/rendering/GpuData.hpp (row-major mat4 layout, vec4-aligned members).
// Only the model matrix is used here — the shadow pass only writes depth.
struct ModelData {
    mat4 model;
    mat4 normalMatrix;
    vec4 emissionUnlit;
    vec4 specularShin;
};

layout(set = 1, binding = 0, row_major, std430) readonly buffer Models {
    ModelData models[];
} modelData;

layout(location = 0) in vec3 inPosition;
layout(location = 4) in int inModelIndex;

void main() {
    bool transformed = inModelIndex >= 0;
    mat4 model = transformed ? modelData.models[inModelIndex].model : mat4(1.0);
    gl_Position = pc.viewProjection * model * vec4(inPosition, 1.0);
}