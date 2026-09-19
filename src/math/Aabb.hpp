#pragma once

#include <limits>

#include "Vect3.hpp"
#include "Mat4.hpp"

// Axis-aligned bounding box. Default-constructed boxes are empty; build a real
// one with fromCenterHalfSize() or expand().
struct Aabb {
	Vect3 min{ std::numeric_limits<float>::infinity(),
	           std::numeric_limits<float>::infinity(),
	           std::numeric_limits<float>::infinity() };
	Vect3 max{ -std::numeric_limits<float>::infinity(),
	           -std::numeric_limits<float>::infinity(),
	           -std::numeric_limits<float>::infinity() };

	Aabb() = default;
	Aabb(const Vect3& min, const Vect3& max) : min(min), max(max) {}

	static Aabb fromCenterHalfSize(const Vect3& center, const Vect3& halfSize);

	Vect3 center() const;
	Vect3 size() const;
	Vect3 halfSize() const;
	float radius() const;

	bool contains(const Vect3& p) const;
	bool intersects(const Aabb& other) const;
	bool empty() const;

	void expand(const Vect3& p);
	void expand(const Aabb& other);

	// Transform the box by an affine matrix and return the tight axis-aligned
	// box around the transformed corners.
	Aabb transformed(const Mat4& transform) const;
};