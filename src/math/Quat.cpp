#include <cmath>

#include "Quat.hpp"

Quat Quat::fromEuler(float yaw, float pitch, float roll) {
	return axisAngle(Vect3(0.0f, 1.0f, 0.0f), yaw) *
	       axisAngle(Vect3(1.0f, 0.0f, 0.0f), pitch) *
	       axisAngle(Vect3(0.0f, 0.0f, -1.0f), roll);
}

Quat Quat::axisAngle(const Vect3& axis, float angle) {
	float len = axis.length();
	if (len < 1e-8f)
		return identity();

	float halfAngle = angle * 0.5f;
	float s = std::sin(halfAngle);

	return Quat(
		axis.x / len * s,
		axis.y / len * s,
		axis.z / len * s,
		std::cos(halfAngle)
	);
}

Quat Quat::fromTwoVectors(const Vect3& from, const Vect3& to) {
	float lenA = from.length();
	float lenB = to.length();
	if (lenA < 1e-8f || lenB < 1e-8f)
		return identity();

	Vect3 a = from * (1.0f / lenA);
	Vect3 b = to * (1.0f / lenB);

	float d = a.dot(b);
	if (d > 0.999999f)
		return identity();
	if (d < -0.999999f)
		return axisAngle(Vect3(1.0f, 0.0f, 0.0f), 3.14159265358979323846f);

	Vect3 axis = a.cross(b);
	Quat q;
	q.x = axis.x;
	q.y = axis.y;
	q.z = axis.z;
	q.w = 1.0f + d;
	return q.normalize();
}

Quat Quat::identity() {
	return Quat(0.0f, 0.0f, 0.0f, 1.0f);
}

Quat Quat::operator*(const Quat& other) const {
	return Quat(
		w * other.x + x * other.w + y * other.z - z * other.y,
		w * other.y - x * other.z + y * other.w + z * other.x,
		w * other.z + x * other.y - y * other.x + z * other.w,
		w * other.w - x * other.x - y * other.y - z * other.z
	);
}

Vect3 Quat::operator*(const Vect3& v) const {
	Vect3 imag(x, y, z);
	Vect3 t = imag.cross(v) * 2.0f;
	return v + t * w + imag.cross(t);
}

Quat Quat::operator*(float scalar) const {
	return Quat(x * scalar, y * scalar, z * scalar, w * scalar);
}

Quat Quat::operator+(const Quat& other) const {
	return Quat(x + other.x, y + other.y, z + other.z, w + other.w);
}

Quat& Quat::operator*=(const Quat& other) {
	*this = (*this) * other;
	return *this;
}

Quat Quat::conjugate() const {
	return Quat(-x, -y, -z, w);
}

float Quat::length() const {
	return std::sqrt(x * x + y * y + z * z + w * w);
}

float Quat::dot(const Quat& other) const {
	return x * other.x + y * other.y + z * other.z + w * other.w;
}

Quat Quat::normalize() const {
	float len = length();
	if (len < 1e-8f)
		return identity();
	return Quat(x / len, y / len, z / len, w / len);
}

Quat Quat::slerp(const Quat& a, const Quat& b, float t) {
	t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);

	float cosTheta = a.dot(b);
	Quat b2 = b;
	if (cosTheta < 0.0f) {
		b2 = b2 * -1.0f;
		cosTheta = -cosTheta;
	}

	if (cosTheta > 0.9995f) {
		Quat r = a * (1.0f - t) + b2 * t;
		return r.normalize();
	}

	float theta = std::acos(cosTheta);
	float sinTheta = std::sin(theta);
	float wa = std::sin((1.0f - t) * theta) / sinTheta;
	float wb = std::sin(t * theta) / sinTheta;

	return Quat(
		a.x * wa + b2.x * wb,
		a.y * wa + b2.y * wb,
		a.z * wa + b2.z * wb,
		a.w * wa + b2.w * wb
	);
}

Mat4 Quat::toMatrix() const {
	Mat4 m;

	float xx = x * x, yy = y * y, zz = z * z;
	float wx = w * x, wy = w * y, wz = w * z;
	float xy = x * y, xz = x * z, yz = y * z;

	m.m[0][0] = 1.0f - 2.0f * (yy + zz);
	m.m[0][1] = 2.0f * (xy - wz);
	m.m[0][2] = 2.0f * (xz + wy);

	m.m[1][0] = 2.0f * (xy + wz);
	m.m[1][1] = 1.0f - 2.0f * (xx + zz);
	m.m[1][2] = 2.0f * (yz - wx);

	m.m[2][0] = 2.0f * (xz - wy);
	m.m[2][1] = 2.0f * (yz + wx);
	m.m[2][2] = 1.0f - 2.0f * (xx + yy);

	return m;
}