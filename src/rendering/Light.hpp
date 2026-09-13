#pragma once

#include <vector>

#include "../math/Vect3.hpp"

// A single light source. `type` chooses which fields are used:
//   Directional — world-space `direction` (ignores position/attenuation).
//   Point       — world-space `position` with `attenuation` falloff.
//   Spot        — world-space `position` shining along `direction`, bounded by
//                 the inner/outer cone cosines (see Light).
struct Light {
    enum class Type : uint32_t {
        Directional = 1,
        Point = 2,
        Spot = 3
    };

    Type type = Type::Directional;

    Vect3 position{ 0.0f, 0.0f, 0.0f };   // used by Point/Spot
    Vect3 direction{ 0.0f, -1.0f, 0.0f }; // used by Directional/Spot

    Vect3 color{ 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;

    // (constant, linear, quadratic) distance falloff for Point/Spot lights.
    Vect3 attenuation{ 1.0f, 0.0f, 0.0f };

    // Spot cone boundaries stored as the cosine of the half-angle,
    // e.g. cos(30°) ≈ 0.866. Fragments inside `spotInnerCos` get full
    // brightness; between inner and outer the light ramps smoothly to zero.
    float spotInnerCos = 0.96f;
    float spotOuterCos = 0.80f;
};

// Global lighting configuration for the current frame. Set
// Renderer::lighting before the frame's draw calls; the engine uploads it to
// the GPU once each frame.
struct LightingState {
    // Ambient light color added uniformly to every lit surface, scaled by the
    // surface's base color. Keeps shadows/back faces from going pure black.
    Vect3 ambientColor{ 0.22f, 0.22f, 0.26f };

    // Active lights this frame. At most 16 are uploaded; extras are ignored.
    std::vector<Light> lights;
};