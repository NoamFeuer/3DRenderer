#include <algorithm>

#include "Aabb.hpp"

Aabb Aabb::fromCenterHalfSize(const Vect3& center, const Vect3& halfSize) {
	Aabb box;
	box.min = center - halfSize;
	box.max = center + halfSize;
	return box;
}

Vect3 Aabb::center() const {
	return (min + max) * 0.5f;
}

Vect3 Aabb::size() const {
	return max - min;
}

Vect3 Aabb::halfSize() const {
	return (max - min) * 0.5f;
}

float Aabb::radius() const {
	return halfSize().length();
}

bool Aabb::contains(const Vect3& p) const {
	return p.x >= min.x && p.x <= max.x &&
	       p.y >= min.y && p.y <= max.y &&
	       p.z >= min.z && p.z <= max.z;
}

bool Aabb::intersects(const Aabb& other) const {
	return min.x <= other.max.x && max.x >= other.min.x &&
	       min.y <= other.max.y && max.y >= other.min.y &&
	       min.z <= other.max.z && max.z >= other.min.z;
}

bool Aabb::empty() const {
	return min.x > max.x || min.y > max.y || min.z > max.z;
}

void Aabb::expand(const Vect3& p) {
	min.x = std::min(min.x, p.x);
	min.y = std::min(min.y, p.y);
	min.z = std::min(min.z, p.z);
	max.x = std::max(max.x, p.x);
	max.y = std::max(max.y, p.y);
	max.z = std::max(max.z, p.z);
}

void Aabb::expand(const Aabb& other) {
	expand(other.min);
	expand(other.max);
}

Aabb Aabb::transformed(const Mat4& transform) const {
	Aabb result;
	if (empty())
		return result;

	for (int i = 0; i < 8; i++) {
		Vect3 corner(
			(i & 1) ? min.x : max.x,
			(i & 2) ? min.y : max.y,
			(i & 4) ? min.z : max.z
		);
		result.expand(transform * corner);
	}
	return result;
}