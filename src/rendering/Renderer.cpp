#include <algorithm>
#include <limits>
#include <vector>
#include "Renderer.hpp"

std::vector<float> depthBuffer;

void clearDepthBuffer(int width, int height) {
    depthBuffer.assign(width * height, std::numeric_limits<float>::infinity());
}

void drawTriangle(SDL_Renderer* renderer, Triangle tri) {
    Vect3 v1 = tri.vertices[0];
    Vect3 v2 = tri.vertices[1];
    Vect3 v3 = tri.vertices[2];

    SDL_RenderDrawLine(renderer, v1.x, v1.y, v2.x, v2.y);
    SDL_RenderDrawLine(renderer, v2.x, v2.y, v3.x, v3.y);
    SDL_RenderDrawLine(renderer, v3.x, v3.y, v1.x, v1.y);
}

void fillTriangle(SDL_Renderer* renderer, Triangle tri, Uint8 r, Uint8 g, Uint8 b) {
    Vect3 v1 = tri.vertices[0];
    Vect3 v2 = tri.vertices[1];
    Vect3 v3 = tri.vertices[2];

    if (v1.y > v2.y) std::swap(v1, v2);
    if (v1.y > v3.y) std::swap(v1, v3);
    if (v2.y > v3.y) std::swap(v2, v3);

    int y1 = (int)v1.y, y2 = (int)v2.y, y3 = (int)v3.y;

    for (int y = y1; y <= y3; y++) {
        float t1 = (y3 == y1) ? 1.0f : (float)(y - y1) / (y3 - y1);
        float xLeft = v1.x + t1 * (v3.x - v1.x);
        float zLeft = v1.z + t1 * (v3.z - v1.z);

        float xRight, zRight;
        if (y < y2) {
            float t2 = (y2 == y1) ? 0.0f : (float)(y - y1) / (y2 - y1);
            xRight = v1.x + t2 * (v2.x - v1.x);
            zRight = v1.z + t2 * (v2.z - v1.z);
        } else {
            float t2 = (y3 == y2) ? 1.0f : (float)(y - y2) / (y3 - y2);
            xRight = v2.x + t2 * (v3.x - v2.x);
            zRight = v2.z + t2 * (v3.z - v2.z);
        }

        if (xLeft > xRight) {
            std::swap(xLeft, xRight);
            std::swap(zLeft, zRight);
        }

        for (int x = (int)xLeft; x <= (int)xRight; x++) {
            float t = (xRight == xLeft) ? 0.0f : (float)(x - xLeft) / (xRight - xLeft);
            float z = zLeft + t * (zRight - zLeft);

            int idx = y * 800 + x;
            if (x >= 0 && x < 800 && y >= 0 && y < 600 && z < depthBuffer[idx]) {
                depthBuffer[idx] = z;
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

Vect3 toScreenSpace(const Vect3& v, int width, int height) {
    return Vect3(
        (v.x + 1.0f) * (width  / 2.0f),
        (1.0f - v.y) * (height / 2.0f),
        v.z
    );
}

struct TriangleToDraw {
    Triangle tri;
    Uint8 r, g, b;
    float depth;
};

void drawMesh(SDL_Renderer* renderer, const Mesh& mesh, const Mat4& proj, const Mat4& transform, int width, int height) {
    Vect3 lightDir = Vect3(0, 0, -1).normalize();
    std::vector<TriangleToDraw> toDraw;

    for (auto& tri : mesh.triangles) {
        Vect3 v1 = transform * tri.vertices[0];
        Vect3 v2 = transform * tri.vertices[1];
        Vect3 v3 = transform * tri.vertices[2];

        Vect3 edge1 = v2 - v1;
        Vect3 edge2 = v3 - v1;
        Vect3 normal = edge1.cross(edge2);

        // backface culling against camera ray
        Vect3 cameraRay = v1;
        if (normal.dot(cameraRay) >= 0) continue;

        // lighting with ambient
        float brightness = normal.normalize().dot(lightDir);
        brightness = 0.1f + brightness * 0.9f;
        if (brightness < 0.1f) brightness = 0.1f;

        Vect3 p1 = toScreenSpace(proj * v1, width, height);
        Vect3 p2 = toScreenSpace(proj * v2, width, height);
        Vect3 p3 = toScreenSpace(proj * v3, width, height);

        Uint8 c = (Uint8)(brightness * 255);

        float avgZ = (v1.z + v2.z + v3.z) / 3.0f;

        Triangle triangle;
        triangle.vertices[0] = p1;
        triangle.vertices[1] = p2;
        triangle.vertices[2] = p3;

        toDraw.push_back({triangle, c, c, c, avgZ});
    }

    std::sort(toDraw.begin(), toDraw.end(), [](const TriangleToDraw& a, const TriangleToDraw& b) {
        return a.depth > b.depth;
    });

    for (auto& t : toDraw) {
        fillTriangle(renderer, t.tri, t.r, t.g, t.b);
    }
}