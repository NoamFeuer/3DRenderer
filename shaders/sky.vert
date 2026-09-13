#version 450

// Sky pass: draws a huge cube centred on the camera. Vertex depth is forced
// to the far plane (gl_Position.xyww) and depth writes are disabled, so the
// sky always renders behind every world object.

layout(push_constant, row_major) uniform PushConstants {
    mat4 viewProjection;
    vec4 skyTop;
    vec4 skyBottom;
} pc;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 fragDir;

void main() {
    gl_Position = (pc.viewProjection * vec4(inPosition, 1.0)).xyww;
    fragDir = inPosition;
}