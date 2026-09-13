#pragma once

#include "../math/Vect3.hpp"

// Material properties for lit, transformed geometry (any draw call that uses a
// model matrix). These are packed per-object into the model storage buffer and
// evaluated every frame in the vertex shader; the fragment shader applies
// them per-pixel together with the light list.
//
// Geometry without a model matrix (raw triangles/rects, text, HUD panels) is
// always unlit: it renders the vertex color as-is, which keeps 2D content
// visually stable regardless of the scene lights.
struct Material {
    // Added to the shaded color before output (use for glows/emissive/UI).
    Vect3 emission{ 0.0f, 0.0f, 0.0f };

    // Blinn-Phong specular tint; a dim gray gives a subtle plastic sheen.
    Vect3 specular{ 0.04f, 0.04f, 0.04f };

    // Blinn-Phong shininess exponent (higher = tighter, glossier highlight).
    float shininess = 24.0f;

    // When true the object still transforms with its model matrix but ignores
    // all lights and renders its base (texture * color) directly — used for
    // sprites/decals that must never look shaded.
    bool unlit = false;
};