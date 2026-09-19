#include "Frustum.hpp"

Frustum Frustum::fromViewProjection(const Mat4& vp) {
	// Gribb & Hartmann plane extraction. Rows are taken directly from the
	// row-major matrix; the Vulkan depth range is [0, 1] so the near plane is
	// the second row on its own instead of the OpenGL-style row2 + row3 combo.
	const float* m = &vp.m[0][0];

	auto row = [&](int i) {
		return Vect3(m[i * 4 + 0], m[i * 4 + 1], m[i * 4 + 2]);
	};

	struct PlaneDef {
		Vect3 normal;
		float distance;
	};

	PlaneDef defs[PlaneCount] = {
		{ row(3) + row(0), vp.m[3][3] + vp.m[0][3] }, // Left
		{ row(3) - row(0), vp.m[3][3] - vp.m[0][3] }, // Right
		{ row(3) + row(1), vp.m[3][3] + vp.m[1][3] }, // Bottom
		{ row(3) - row(1), vp.m[3][3] - vp.m[1][3] }, // Top
		{ row(2),        vp.m[2][3] },               // Near
		{ row(3) - row(2), vp.m[3][3] - vp.m[2][3] }, // Far
	};

	Frustum f;
	for (int i = 0; i < PlaneCount; i++) {
		float len = defs[i].normal.length();
		// A degenerate extraction (e.g. an all-zero row) leaves the plane
		// unusable; scale it so it never culls anything.
		if (len < 1e-12f) {
			f.normals[i] = Vect3(0.0f, 0.0f, 0.0f);
			f.distances[i] = 1.0f;
			continue;
		}
		f.normals[i] = defs[i].normal * (1.0f / len);
		f.distances[i] = defs[i].distance / len;
	}
	return f;
}

bool Frustum::contains(const Vect3& point) const {
	for (int i = 0; i < PlaneCount; i++) {
		if (planeDist(point, i) < 0.0f)
			return false;
	}
	return true;
}

bool Frustum::intersects(const Aabb& box) const {
	for (int i = 0; i < PlaneCount; i++) {
		// Find the corner furthest along the plane normal; if even that corner
		// is behind the plane the whole box is outside it.
		Vect3 positive = box.min;
		if (normals[i].x >= 0.0f) positive.x = box.max.x;
		if (normals[i].y >= 0.0f) positive.y = box.max.y;
		if (normals[i].z >= 0.0f) positive.z = box.max.z;

		if (positive.dot(normals[i]) + distances[i] < 0.0f)
			return false;
	}
	return true;
}