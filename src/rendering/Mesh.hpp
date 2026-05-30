#pragma once

#include "../math/Vect3.hpp"
#include <vector>

struct Triangle {
	Vect3 vertices[3];
};

struct Mesh {
	std::vector<Triangle> triangles;
};

Mesh createCube();
