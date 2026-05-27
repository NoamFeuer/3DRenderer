#include <SDL2/SDL.h>
#include <cmath>
#include "rendering/Mesh.hpp"
#include "rendering/Renderer.hpp"
#include "math/Mat4.hpp"

const float FOV = 90.0f;
const int SCREEN_WIDTH = 800.0f;
const int SCREEN_HEIGHT = 600.0f;

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow(
		"3D Rendering Engine",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN
	);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

	Mesh cube = createCube();
	Mat4 proj = Mat4::projection(FOV * M_PI/180.0f, 800.0f/600.0f, 0.1f, 100.0f);
	Mat4 trans = Mat4::translation(-0.5f, -0.5f, 3.0f);

	float angle = 0.0f;
	bool running = true;
	SDL_Event e;

	while (running) {
		while (SDL_PollEvent(&e))
			if (e.type == SDL_QUIT) running = false;

		angle += 0.002f;

		Mat4 center = Mat4::translation(-0.5f, -0.5f, -0.5f);
		Mat4 rotY = Mat4::rotationY(angle);
		Mat4 trans = Mat4::translation(0.0f, 0.0f, 3.0f);

		Mat4 transform = trans * rotY * center; 

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		drawMesh(renderer, cube, proj, transform, SCREEN_WIDTH, SCREEN_HEIGHT);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
