#pragma once

#include"vect3.hpp"

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
	static Mat4 projection(float fov, float aspect, float near, float far);
};
