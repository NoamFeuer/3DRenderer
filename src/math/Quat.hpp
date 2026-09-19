#pragma once

#include "Mat4.hpp"
#include "Vect3.hpp"

// Unit quaternion with (x, y, z, w) layout - x/y/z imaginary part, w real.
// Represents a rotation; identity by default. convert to a rotation matrix
// with toMatrix() or rotate vectors with operator*.
struct Quat {
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;

	Quat() = default;
	Quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	// yaw rotates around +Y, pitch around +X, roll around -Z; the result is
	// R = RotationY(yaw) * RotationX(pitch) * RotationZ(roll).
	static Quat fromEuler(float yaw, float pitch, float roll);

	static Quat axisAngle(const Vect3& axis, float angle);
	static Quat fromTwoVectors(const Vect3& from, const Vect3& to);

	static Quat identity();

	Quat operator*(const Quat& other) const;
	Vect3 operator*(const Vect3& v) const;
	Quat operator*(float scalar) const;
	Quat operator+(const Quat& other) const;
	Quat& operator*=(const Quat& other);

	Quat conjugate() const;
	float length() const;
	float dot(const Quat& other) const;
	Quat normalize() const;

	// Spherical linear interpolation between two unit quaternions.
	static Quat slerp(const Quat& a, const Quat& b, float t);

	Vect3 rotate(const Vect3& v) const { return (*this) * v; }

	Mat4 toMatrix() const;

	Vect3 getForward() const { return rotate(Vect3(0.0f, 0.0f, -1.0f)); }
	Vect3 getRight() const { return rotate(Vect3(1.0f, 0.0f, 0.0f)); }
	Vect3 getUp() const { return rotate(Vect3(0.0f, 1.0f, 0.0f)); }
};