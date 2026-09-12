#version 450

layout(push_constant, row_major) uniform PushConstants {
    mat4 viewProjection;
} pc;

// Per-object transforms. Matches Mat4's row-major CPU layout; modelIndex < 0
// selects identity so untransformed geometry needs no buffer entry. `readonly`
// marks it NonWritable so it can be used from the vertex stage.
layout(set = 1, binding = 0, row_major, std430) readonly buffer ModelMatrices {
    mat4 modelMatrices[];
} models;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in int inTexIndex;
layout(location = 4) in int inModelIndex;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out flat int fragTexIndex;

void main() {
    mat4 model = (inModelIndex >= 0) ? models.modelMatrices[inModelIndex] : mat4(1.0);
    gl_Position = pc.viewProjection * model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragUV = inUV;
    fragTexIndex = inTexIndex;
}