#include <iostream>
#include "Engine.hpp"

int main() {
    try {
        Engine engine(800, 800, "Feast Your Eyes!");

        engine.run([](float deltaTime) {
            // Per-frame user logic goes here later (input, updates, etc.)
            (void)deltaTime;
        });
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}
