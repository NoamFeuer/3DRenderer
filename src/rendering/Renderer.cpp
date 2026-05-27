#include <algorithm>
#include"Renderer.hpp"

void drawTriangle(SDL_Renderer* renderer, Vect3 v1, Vect3 v2, Vect3 v3) {
	SDL_RenderDrawLine(renderer, v1.x, v1.y, v2.x, v2.y);
	SDL_RenderDrawLine(renderer, v2.x, v2.y, v3.x, v3.y);
	SDL_RenderDrawLine(renderer, v3.x, v3.y, v1.x, v1.y);
}

void fillTriangle(SDL_Renderer* renderer, Vect3 v1, Vect3 v2, Vect3 v3, Uint8 r, Uint8 g, Uint8 b) {
    // sort vertices by y (v1 top, v3 bottom)
    if (v1.y > v2.y) std::swap(v1, v2);
    if (v1.y > v3.y) std::swap(v1, v3);
    if (v2.y > v3.y) std::swap(v2, v3);

    int y1 = (int)v1.y, y2 = (int)v2.y, y3 = (int)v3.y;

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);

    for (int y = y1; y <= y3; y++) {
        // left and right x for this scanline
        float t1 = (y3 == y1) ? 1.0f : (float)(y - y1) / (y3 - y1);
        float xLeft = v1.x + t1 * (v3.x - v1.x);

        float xRight;
        if (y < y2) {
            float t2 = (y2 == y1) ? 0.0f : (float)(y - y1) / (y2 - y1);
            xRight = v1.x + t2 * (v2.x - v1.x);
        } else {
            float t2 = (y3 == y2) ? 1.0f : (float)(y - y2) / (y3 - y2);
            xRight = v2.x + t2 * (v3.x - v2.x);
        }

        if (xLeft > xRight) std::swap(xLeft, xRight);
        SDL_RenderDrawLine(renderer, (int)xLeft, y, (int)xRight, y);
    }
}

Vect3 toScreenSpace(const Vect3& v, int width, int height) {
	return Vect3(
		(v.x + 1.0f) * (width  / 2.0f),
		(1.0f - v.y) * (height / 2.0f),
		v.z
	);
}

void drawMesh(SDL_Renderer* renderer, const Mesh& mesh, const Mat4& proj, const Mat4& transform, int width, int height) {
	Vect3 lightDir = Vect3(0, 0, -1).normalize();

	for (auto& tri : mesh.triangles) {
		// transform vertices
		Vect3 v1 = transform * tri.vertices[0];
		Vect3 v2 = transform * tri.vertices[1];
		Vect3 v3 = transform * tri.vertices[2];

		// calculate normal
		Vect3 edge1 = v2 - v1;
		Vect3 edge2 = v3 - v1;
		Vect3 normal = edge1.cross(edge2);

		// backface culling
		if (normal.z >= 0) continue;

		// lighting
		float brightness = normal.normalize().dot(lightDir);
		if (brightness < 0) brightness = 0;

		// project to screen space
		Vect3 p1 = toScreenSpace(proj * v1, width, height);
		Vect3 p2 = toScreenSpace(proj * v2, width, height);
		Vect3 p3 = toScreenSpace(proj * v3, width, height);

		// draw with brightness
		Uint8 c = (Uint8)(brightness * 255);
		fillTriangle(renderer, p1, p2, p3, c, c, c);
	}
}
