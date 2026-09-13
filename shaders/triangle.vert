#version 450

layout(push_constant, row_major) uniform PushConstants {
    mat4 viewProjection;
} pc;

// Per-object transform + material. Matches the CPU-side ModelData struct in
// src/rendering/GpuData.hpp (row-major mat4 layout, vec4-aligned members).
// modelIndex < 0 selects identity/untagged geometry.
struct ModelData {
    mat4 model;
    mat4 normalMatrix;
    vec4 emissionUnlit;  // rgb emission, a = 1.0 when lit
    vec4 specularShin;   // rgb specular, a = shininess
};

layout(set = 1, binding = 0, row_major, std430) readonly buffer Models {
    ModelData models[];
} modelData;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in int inTexIndex;
layout(location = 4) in int inModelIndex;
layout(location = 5) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out flat int fragTexIndex;
layout(location = 3) out vec3 fragWorld;
layout(location = 4) out vec3 fragNormal;
layout(location = 5) out flat vec4 fragEmission;
layout(location = 6) out flat vec4 fragSpecShin;
layout(location = 7) out flat int fragLit;

void main() {
    bool transformed = inModelIndex >= 0;
    vec4 worldPos;
    vec3 worldNormal;

    if (transformed) {
        ModelData md = modelData.models[inModelIndex];
        worldPos = md.model * vec4(inPosition, 1.0);
        worldNormal = mat3(md.normalMatrix) * inNormal;
        fragEmission = md.emissionUnlit;
        fragSpecShin = md.specularShin;
        fragLit = (md.emissionUnlit.a > 0.5) ? 1 : 0;
    } else {
        worldPos = vec4(inPosition, 1.0);
        worldNormal = inNormal;
        fragEmission = vec4(0.0);
        fragSpecShin = vec4(0.0);
        fragLit = 0;
    }

    gl_Position = pc.viewProjection * worldPos;
    fragColor = inColor;
    fragUV = inUV;
    fragTexIndex = inTexIndex;
    fragWorld = worldPos.xyz;
    fragNormal = worldNormal;
}