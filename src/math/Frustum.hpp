#pragma once

#include <cmath>

#include "Vect3.hpp"
#include "Mat4.hpp"
#include "Aabb.hpp"

// View frustum defined by six world-space planes extracted from the combined
// view-projection matrix. A point is inside the frustum iff it is on the
// positive side of every plane: dot(normal, p) + distance >= 0.
struct Frustum {
	enum Plane {
		Left,
		Right,
		Bottom,
		Top,
		Near,
		Far,
		PlaneCount
	};

	Vect3 normals[PlaneCount];
	float distances[PlaneCount];

	static Frustum fromViewProjection(const Mat4& viewProjection);

	bool contains(const Vect3& point) const;
	// Conservative test: a box is rejected only when every corner lies behind
	// a single plane. Never falsely culls, may accept boxes at the edges.
	bool intersects(const Aabb& box) const;

private:
	float planeDist(const Vect3& point, int plane) const {
		return point.dot(normals[plane]) + distances[plane];
	}
};