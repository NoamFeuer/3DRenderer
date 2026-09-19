#include <cmath>
#include <limits>

#include "Ray.hpp"

namespace {
	float clamp01(float x) {
		return x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x);
	}
}

Ray Ray::betweenPoints(const Vect3& a, const Vect3& b) {
	Vect3 d = b - a;
	float len = d.length();
	Ray r;
	r.origin = a;
	r.direction = len > 1e-8f ? d * (1.0f / len) : Vect3(0.0f, 0.0f, 1.0f);
	return r;
}

Ray Ray::fromNdc(const Mat4& inverseViewProjection, float ndcX, float ndcY) {
	// Clip-space z = 0 (near) and z = 1 (far) both use a w of 1 here, which
	// Mat4::operator*(Vect3) applies implicitly.
	Vect3 nearPoint = inverseViewProjection * Vect3(ndcX, ndcY, 0.0f);
	Vect3 farPoint = inverseViewProjection * Vect3(ndcX, ndcY, 1.0f);
	return betweenPoints(nearPoint, farPoint);
}

Vect3 Ray::pointAt(float t) const {
	return origin + direction * t;
}

bool Ray::intersectsPlane(const Vect3& pointOnPlane, const Vect3& planeNormal, float& tOut) const {
	float denom = direction.dot(planeNormal);
	if (std::abs(denom) < 1e-8f)
		return false;

	tOut = (pointOnPlane - origin).dot(planeNormal) / denom;
	return tOut >= 0.0f;
}

bool Ray::intersectsSphere(const Vect3& center, float radius, float& tOut) const {
	Vect3 oc = origin - center;
	float b = oc.dot(direction);
	float c = oc.dot(oc) - radius * radius;
	float disc = b * b - c;
	if (disc < 0.0f)
		return false;

	float sqrtDisc = std::sqrt(disc);
	float t = -b - sqrtDisc;
	if (t < 0.0f)
		t = -b + sqrtDisc;
	if (t < 0.0f)
		return false;

	tOut = t;
	return true;
}

bool Ray::intersectsAabb(const Aabb& box, float& tMinOut, float& tMaxOut) const {
	float tMin = -std::numeric_limits<float>::infinity();
	float tMax = std::numeric_limits<float>::infinity();

	for (int axis = 0; axis < 3; axis++) {
		float o = (axis == 0) ? origin.x : (axis == 1) ? origin.y : origin.z;
		float d = (axis == 0) ? direction.x : (axis == 1) ? direction.y : direction.z;
		float lo = (axis == 0) ? box.min.x : (axis == 1) ? box.min.y : box.min.z;
		float hi = (axis == 0) ? box.max.x : (axis == 1) ? box.max.y : box.max.z;

		if (std::abs(d) < 1e-8f) {
			if (o < lo || o > hi)
				return false;
			continue;
		}

		float invD = 1.0f / d;
		float t1 = (lo - o) * invD;
		float t2 = (hi - o) * invD;
		if (t1 > t2)
			std::swap(t1, t2);

		if (t1 > tMin) tMin = t1;
		if (t2 < tMax) tMax = t2;
		if (tMin > tMax)
			return false;
	}

	tMinOut = tMin;
	tMaxOut = tMax;
	return true;
}

float Ray::closestDistanceToPoint(const Vect3& point) const {
	float t = clamp01((point - origin).dot(direction));
	return (pointAt(t) - point).length();
}