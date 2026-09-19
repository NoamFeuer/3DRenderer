#include <cmath>
#include <utility>
#include "Mat4.hpp"

Mat4::Mat4() {
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			m[i][j] = (i == j) ? 1.0f : 0.0f;
}

Mat4 Mat4::identity() {
	return Mat4();
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

	// Vulkan NDC has Y pointing down and a depth range of [0,1], so the near
	// plane maps to 0 and the far plane to 1 (w = -eyeZ keeps the depth sign
	// in line with the -Z viewing direction). The negative Y terms flip the
	// projection so that world +Y (up) lands at the top of the rendered
	// image instead of being mirrored vertically. Without this the whole
	// scene renders upside down on a Vulkan swapchain.
	m.m[0][0] = 1.0f / (aspect * tanHalfFov);
	m.m[1][1] = -1.0f / tanHalfFov;
	m.m[2][2] = far / (near - far);
	m.m[2][3] = (far * near) / (near - far);
	m.m[3][2] = -1.0f;

	return m;
}

Mat4 Mat4::inverse() const {
	Mat4 a = *this;
	Mat4 inv;

	for (int col = 0; col < 4; col++) {
		int pivot = col;
		float maxAbs = std::fabs(a.m[col][col]);
		for (int row = col + 1; row < 4; row++) {
			float v = std::fabs(a.m[row][col]);
			if (v > maxAbs) {
				maxAbs = v;
				pivot = row;
			}
		}

		if (maxAbs < 1e-12f)
			return Mat4();

		if (pivot != col) {
			for (int j = 0; j < 4; j++) {
				std::swap(a.m[pivot][j], a.m[col][j]);
				std::swap(inv.m[pivot][j], inv.m[col][j]);
			}
		}

		float d = a.m[col][col];
		for (int j = 0; j < 4; j++) {
			a.m[col][j] /= d;
			inv.m[col][j] /= d;
		}

		for (int row = 0; row < 4; row++) {
			if (row == col)
				continue;
			float f = a.m[row][col];
			if (f == 0.0f)
				continue;
			for (int j = 0; j < 4; j++) {
				a.m[row][j] -= f * a.m[col][j];
				inv.m[row][j] -= f * inv.m[col][j];
			}
		}
	}

	return inv;
}

Mat4 Mat4::ortho(float left, float right, float bottom, float top, float near, float far) {
	Mat4 m;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			m.m[i][j] = 0;

	m.m[0][0] = 2.0f / (right - left);
	m.m[1][1] = 2.0f / (top - bottom);
	m.m[2][2] = 1.0f / (near - far);
	m.m[0][3] = -(right + left) / (right - left);
	m.m[1][3] = -(top + bottom) / (top - bottom);
	m.m[2][3] = near / (near - far);
	m.m[3][3] = 1.0f;

	return m;
}

