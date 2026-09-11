#version 450

layout(push_constant, row_major) uniform PushConstants {
    mat4 viewProjection;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in int inTexIndex;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out flat int fragTexIndex;

void main() {
    gl_Position = pc.viewProjection * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragUV = inUV;
    fragTexIndex = inTexIndex;
}