#include "vect3.hpp"

Vect3& Vect3::operator=(const Vect3& other)
{
	if (this == &other) return *this;

	x = other.x;
	y = other.y;
	z = other.z;

	return *this;
}

Vect3 Vect3::operator+(const Vect3& other) const {
	return Vect3(
		x + other.x,
		y + other.y,
		z + other.z
	);
}

Vect3 Vect3::operator-(const Vect3& other) const {
	return Vect3(
		x - other.x,
		y - other.y,
		z - other.z
	);
}

Vect3 Vect3::operator*(const Vect3& other) const {
	return Vect3(
		x * other.x,
		y * other.y,
		z * other.z
	);
}

Vect3 Vect3::operator*(float scalar) const {
	return Vect3(
		x * scalar,
		y * scalar,
		z * scalar
	);
}

Vect3& Vect3::operator+=(const Vect3& other) {
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

Vect3& Vect3::operator-=(const Vect3& other) {
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}

Vect3& Vect3::operator*=(const Vect3& other) {
	x *= other.x;
	y *= other.y;
	z *= other.z;

	return *this;
}

Vect3& Vect3::operator*=(float scalar) {
	x *= scalar;
	y *= scalar;
	z *= scalar;

	return *this;
}

Vect3 Vect3::operator-() const {
	return Vect3(
		-x,
		-y,
		-z
	);
}

Vect3 operator*(float scalar, const Vect3& v) {
	return Vect3(
		v.x * scalar,
		v.y * scalar,
		v.z * scalar
	);
}

std::ostream& operator<<(std::ostream& os, const Vect3& v) {
	os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
	return os;
}

float Vect3::dot(const Vect3& other) const {
	return x * other.x + y * other.y + z * other.z;
}

Vect3 Vect3::cross(const Vect3& other) const {
	return Vect3(
		y * other.z - z * other.y,
		z * other.x - x * other.z,
		x * other.y - y * other.x
	);
}

float Vect3::length() const {
	return sqrt(x*x + y*y + z*z);
}

float Vect3::lengthSquared() const {
	return (x*x + y*y + z*z);
}

Vect3 Vect3::normalize() const {
	float len = length();
	return Vect3(x/len, y/len, z/len);
}
