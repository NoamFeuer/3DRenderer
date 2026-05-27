#pragma once

#include <SDL2/SDL.h>
#include "Mesh.hpp"
#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"

void drawTriangle(SDL_Renderer* renderer, Vect3 v1, Vect3 v2, Vect3 v3);
void drawMesh(SDL_Renderer* renderer, const Mesh& mesh, const Mat4& proj, const Mat4& transform, int w, int h);
void fillTriangle(SDL_Renderer* renderer, Vect3 v1, Vect3 v2, Vect3 v3, Uint8 r, Uint8 g, Uint8 b);
Vect3 toScreenSpace(const Vect3& v, int width, int height);
