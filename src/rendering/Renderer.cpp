#include "Renderer.hpp"

void drawTriangle(SDL_Renderer* renderer, Vect3 v1, Vect3 v2, Vect3 v3) {
    SDL_RenderDrawLine(renderer, v1.x, v1.y, v2.x, v2.y);
    SDL_RenderDrawLine(renderer, v2.x, v2.y, v3.x, v3.y);
    SDL_RenderDrawLine(renderer, v3.x, v3.y, v1.x, v1.y);
}

Vect3 toScreenSpace(const Vect3& v, int width, int height) {
    return Vect3(
        (v.x + 1.0f) * (width  / 2.0f),
        (1.0f - v.y) * (height / 2.0f),
        v.z
    );
}

void drawMesh(SDL_Renderer* renderer, const Mesh& mesh, const Mat4& proj, const Mat4& transform, int width, int height) {
    for (auto& tri : mesh.triangles) {
        // transform vertices
        Vect3 v1 = transform * tri.vertices[0];
        Vect3 v2 = transform * tri.vertices[1];
        Vect3 v3 = transform * tri.vertices[2];

        // calculate normal
        Vect3 edge1 = v2 - v1;
        Vect3 edge2 = v3 - v1;
        Vect3 normal = edge1.cross(edge2);

        // backface culling: skip if facing away from camera
        if (normal.z >= 0) continue;

        // project to screen space
        Vect3 p1 = toScreenSpace(proj * v1, width, height);
        Vect3 p2 = toScreenSpace(proj * v2, width, height);
        Vect3 p3 = toScreenSpace(proj * v3, width, height);

        drawTriangle(renderer, p1, p2, p3);
    }
}
