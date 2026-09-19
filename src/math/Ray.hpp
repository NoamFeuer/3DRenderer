#pragma once

#include "Vect3.hpp"
#include "Mat4.hpp"
#include "Aabb.hpp"

struct Ray {
	Vect3 origin;
	Vect3 direction; // normalized

	Ray() = default;
	Ray(const Vect3& origin, const Vect3& direction)
		: origin(origin), direction(direction) {}

	static Ray betweenPoints(const Vect3& a, const Vect3& b);
	// Build a world-space ray from normalized device coordinates in [-1, 1].
	// inverseViewProjection must be the inverse of projection * view.
	static Ray fromNdc(const Mat4& inverseViewProjection, float ndcX, float ndcY);

	Vect3 pointAt(float t) const;

	bool intersectsPlane(const Vect3& pointOnPlane, const Vect3& planeNormal, float& tOut) const;
	bool intersectsSphere(const Vect3& center, float radius, float& tOut) const;
	// Slab test. Returns true if the ray hits the box at all; tMinOut/tMaxOut
	// receive the entry/exit parameters along the ray (can be negative when
	// the origin is inside or behind the box).
	bool intersectsAabb(const Aabb& box, float& tMinOut, float& tMaxOut) const;

	float closestDistanceToPoint(const Vect3& point) const;
};