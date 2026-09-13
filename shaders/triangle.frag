#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(set = 0, binding = 0) uniform sampler2D textures[];

struct GpuLight {
    vec4 positionType;     // rgb position, a = type (see Light::Type)
    vec4 directionOuter;   // rgb direction, a = spot outer cos
    vec4 colorIntensity;   // rgb color, a = intensity
    vec4 attenuationInner; // rgb (constant, linear, quadratic), a = spot inner cos
};

layout(set = 2, binding = 0) uniform Lighting {
    vec4 cameraPosition;
    vec4 ambient;
    vec4 counts;           // x = number of active lights
    GpuLight lights[16];
} lighting;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in flat int fragTexIndex;
layout(location = 3) in vec3 fragWorld;
layout(location = 4) in vec3 fragNormal;
layout(location = 5) in flat vec4 fragEmission;
layout(location = 6) in flat vec4 fragSpecShin;
layout(location = 7) in flat int fragLit;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 texColor = (fragTexIndex >= 0)
        ? texture(nonuniformEXT(textures[fragTexIndex]), fragUV)
        : vec4(1.0);
    vec4 base = texColor * vec4(fragColor, 1.0);

    // Untagged / unlit geometry (raw primitives, sprites, text, HUD) is never
    // affected by the scene lights — it renders its base color directly.
    if (fragLit == 0) {
        outColor = base;
        return;
    }

    vec3 N = normalize(fragNormal);
    vec3 V = normalize(lighting.cameraPosition.xyz - fragWorld);

    vec3 color = base.rgb * lighting.ambient.rgb;

    int lightCount = int(lighting.counts.x);
    for (int i = 0; i < lightCount; i++) {
        GpuLight L = lighting.lights[i];
        int type = int(L.positionType.a);

        vec3 lightDir;
        float attenuation = 1.0;
        float spotFactor = 1.0;

        if (type == 1) {                // directional
            lightDir = -L.directionOuter.xyz;
        } else {                        // point or spot
            vec3 toLight = L.positionType.xyz - fragWorld;
            float dist = length(toLight);
            lightDir = toLight / max(dist, 1e-4);
            vec3 att = L.attenuationInner.xyz;
            attenuation = 1.0 / (att.x + att.y * dist + att.z * dist * dist);

            if (type == 3) {            // spot
                float cosAngle = dot(-lightDir, normalize(L.directionOuter.xyz));
                float inner = L.attenuationInner.a;
                float outer = L.directionOuter.a;
                float t = clamp((cosAngle - outer) / max(inner - outer, 1e-4), 0.0, 1.0);
                spotFactor = t * t;
            }
        }

        float NdotL = max(dot(N, lightDir), 0.0);
        vec3 diffuse = base.rgb * L.colorIntensity.rgb * L.colorIntensity.a
                     * NdotL * attenuation * spotFactor;

        vec3 H = normalize(lightDir + V);
        float NdotH = max(dot(N, H), 0.0);
        float specPower = pow(NdotH, fragSpecShin.a);
        vec3 specular = fragSpecShin.rgb * L.colorIntensity.rgb * L.colorIntensity.a
                      * specPower * attenuation * spotFactor;

        color += diffuse + specular;
    }

    color += fragEmission.rgb;          // emissive materials glow on top

    outColor = vec4(color, base.a);
}