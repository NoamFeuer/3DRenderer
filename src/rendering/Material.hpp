#pragma once

#include "../math/Vect3.hpp"

struct Material {
    Vect3 emission{ 0.0f, 0.0f, 0.0f };
    Vect3 specular{ 0.04f, 0.04f, 0.04f };

    float shininess = 24.0f;

    bool unlit = false;
};