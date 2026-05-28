#include<cmath>
#include"Mat4.hpp"

Mat4::Mat4() {
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			m[i][j] = (i == j) ? 1.0f : 0.0f;
}

Mat4 Mat4::operator*(const Mat4& other) const {
	Mat4 result;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++) {
			result.m[i][j] = 0;
			for (int k = 0; k < 4; k++)
				result.m[i][j] += m[i][k] * other.m[k][j];
		}
	return result;
}

Vect3 Mat4::operator*(const Vect3& v) const {
	float x = m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3];
	float y = m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3];
	float z = m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3];
	float w = m[3][0]*v.x + m[3][1]*v.y + m[3][2]*v.z + m[3][3];

	// Divide by w for perspective correct projection
	if (w != 0) return Vect3(x/w, y/w, z/w);
	return Vect3(x, y, z);
}

Mat4 Mat4::translation(float x, float y, float z) {
	Mat4 m;
	m.m[0][3] = x;
	m.m[1][3] = y;
	m.m[2][3] = z;

	return m;
}

Mat4 Mat4::scale(float x, float y, float z) {
	Mat4 m;
	m.m[0][0] = x;
	m.m[1][1] = y;
	m.m[2][2] = z;

	return m;
}

Mat4 Mat4::rotationX(float angle) {
	Mat4 m;

	float c = cos(angle);
	float s = sin(angle);

	m.m[1][1] = c;
	m.m[1][2] = -s;
	m.m[2][1] = s;
	m.m[2][2] = c;

	return m;
}

Mat4 Mat4::rotationY(float angle) {
    Mat4 m;

    float c = cos(angle);
    float s = sin(angle);

    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;

    return m;
}

Mat4 Mat4::rotationZ(float angle) {
    Mat4 m;

    float c = cos(angle);
    float s = sin(angle);

    m.m[0][0] = c;
    m.m[0][1] = -s;
    m.m[1][0] = s;
    m.m[1][1] = c;

    return m;
}

Mat4 Mat4::projection(float fov, float aspect, float near, float far) {
	Mat4 m;

	// Zero out the matrix first
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			m.m[i][j] = 0;

	float tanHalfFov = tan(fov / 2.0f);

	m.m[0][0] = 1.0f / (aspect * tanHalfFov);
	m.m[1][1] = 1.0f / tanHalfFov;
	m.m[2][2] = -(far + near) / (far - near);
	m.m[2][3] = -(2.0f * far * near) / (far - near);
	m.m[3][2] = -1.0f;

	return m;
}

