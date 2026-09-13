#version 450

// Procedural gradient sky. The color depends only on the view direction, so
// it needs no texture: dir.y near the horizon blends toward skyBottom and
// rises to skyTop overhead / dips below the horizon into the bottom color.

layout(push_constant, row_major) uniform PushConstants {
    mat4 viewProjection;
    vec4 skyTop;
    vec4 skyBottom;
} pc;

layout(location = 0) in vec3 fragDir;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 dir = normalize(fragDir);

    // 0 below the horizon, 1 directly overhead.
    float up = clamp(dir.y, 0.0, 1.0);

    // Soften the transition so the horizon isn't a hard line.
    float horizonGlow = 1.0 - up;
    horizonGlow = pow(horizonGlow, 3.0);

    vec3 color = mix(pc.skyBottom.rgb, pc.skyTop.rgb, up);

    // Fade to a pale haze right at the horizon line then straight to the
    // bottom color below it.
    float h = smoothstep(0.0, 0.12, dir.y);
    color = mix(pc.skyBottom.rgb, color, h);

    outColor = vec4(color, 1.0);
}