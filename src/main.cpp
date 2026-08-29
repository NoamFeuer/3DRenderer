#include <iostream>
#include "Engine.hpp"
#include "math/Vect3.hpp"

int main() {
    try {
        Engine engine(800, 600, "My Engine");

        engine.run([](float deltaTime, Renderer& renderer) {
            (void)deltaTime;

            renderer.beginFrame();

            renderer.drawRectangle(
                Vect3(-0.5f, -0.5f, 0.0f),
                Vect3(0.5f, -0.5f, 0.0f),
                Vect3(0.5f, 0.5f, 0.0f),
                Vect3(-0.5f, 0.5f, 0.0f),
                Vect3(1.0f, 1.0f, 1.0f));
        });
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}
