#pragma once
#include <cmath>
#include <ostream>

struct Vect3 {
	float x, y, z;

	// Default constructor
	Vect3() : x(0), y(0), z(0) {}

	// Parameterized constructor
	Vect3(float x, float y, float z) : x(x), y(y), z(z) {}

	// Copy constructor
	Vect3(const Vect3& other) : x(other.x), y(other.y), z(other.z) {}

	// Copy assignment operator
	Vect3& operator=(const Vect3& other);

	// Other operators
	Vect3 operator+(const Vect3& other) const;
	Vect3 operator-(const Vect3& other) const;
	Vect3 operator*(const Vect3& other) const;
	Vect3 operator*(float scalar) const;

	Vect3& operator+=(const Vect3& other);
	Vect3& operator-=(const Vect3& other);
	Vect3& operator*=(const Vect3& other);
	Vect3& operator*=(float scalar);

	Vect3 operator-() const;

	// Some handy functions
	float dot(const Vect3& other) const;
	Vect3 cross(const Vect3& other) const;
	float length() const;
	float lengthSquared() const;
	Vect3 normalize() const;
};

// More operators. scalar*v and printing
Vect3 operator*(float scalar, const Vect3& v);
std::ostream& operator<<(std::ostream& os, const Vect3& v);
