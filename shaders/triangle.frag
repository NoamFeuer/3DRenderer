#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler2D textures[];

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in flat int fragTexIndex;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 texColor = (fragTexIndex >= 0)
        ? texture(nonuniformEXT(textures[fragTexIndex]), fragUV)
        : vec4(1.0);
    outColor = texColor * vec4(fragColor, 1.0);
}