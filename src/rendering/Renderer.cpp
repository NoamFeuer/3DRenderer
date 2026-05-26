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

void drawMesh(SDL_Renderer* renderer, const Mesh& mesh, const Mat4& proj, const Mat4& transform, const int w, const int h) {
    for (auto& tri : mesh.triangles) {
        Vect3 v1 = toScreenSpace(proj * (transform * tri.vertices[0]), w, h);
        Vect3 v2 = toScreenSpace(proj * (transform * tri.vertices[1]), w, h);
        Vect3 v3 = toScreenSpace(proj * (transform * tri.vertices[2]), w, h);

        drawTriangle(renderer, v1, v2, v3);
    }
}
