#pragma once

#include <vector>

#include "../math/Vect3.hpp"

struct Light {
    enum class Type : uint32_t {
        Directional = 1,
        Point = 2,
        Spot = 3
    };

    Type type = Type::Directional;

    Vect3 position{ 0.0f, 0.0f, 0.0f };
    Vect3 direction{ 0.0f, -1.0f, 0.0f };

    Vect3 color{ 1.0f, 1.0f, 1.0f };
    float intensity = 1.0f;

    Vect3 attenuation{ 1.0f, 0.0f, 0.0f };

    float spotInnerCos = 0.96f;
    float spotOuterCos = 0.80f;
};

struct LightingState {
    Vect3 ambientColor{ 0.22f, 0.22f, 0.26f };

    std::vector<Light> lights;
};