#pragma once

#include "Vect3.hpp"

struct Mat4 {
	float m[4][4];

	// Constructors
	Mat4(); // Identity matrix
	Mat4(float m[4][4]);

	// Operators
	Mat4 operator*(const Mat4& other) const;
	Vect3 operator*(const Vect3& v) const; // transform a vector

	// Static factory functions
	static Mat4 identity();
	static Mat4 translation(float x, float y, float z);
	static Mat4 rotationX(float angle);
	static Mat4 rotationY(float angle);
	static Mat4 rotationZ(float angle);
	static Mat4 scale(float x, float y, float z);

	// Perspective projection. Cameras look down -Z; `near`/`far` are positive
	// distances along that axis. Depth is mapped into Vulkan's [0,1] NDC range:
	// near plane -> 0, far plane -> 1.
	static Mat4 projection(float fov, float aspect, float near, float far);

	// Orthographic projection with the same -Z viewing/depth convention as
	// projection() (near -> 0, far -> 1 in NDC). left/right/bottom/top are the
	// world-space bounds at the near plane.
	static Mat4 ortho(float left, float right, float bottom, float top, float near, float far);
};
