#include <iostream>
#include "../Engine.hpp"
#include "../math/Vect3.hpp"
#include "../math/Mat4.hpp"

int run() {
    try {
        Engine engine(800, 600, "My Engine");

        const float moveSpeed = 3.0f;
        const float mouseSensitivity = 0.0025f;

        engine.run([&](float deltaTime, Renderer& renderer, Camera& camera, Input& input) {
            camera.yaw -= input.getMouseDeltaX() * mouseSensitivity;
            camera.pitch += input.getMouseDeltaY() * mouseSensitivity;

            const float maxPitch = 1.5f;
            if (camera.pitch > maxPitch) camera.pitch = maxPitch;
            if (camera.pitch < -maxPitch) camera.pitch = -maxPitch;

            Vect3 forward = camera.getForward();
            Vect3 right = camera.getRight();

            if (input.isKeyDown(Key::W)) camera.position += forward * (moveSpeed * deltaTime);
            if (input.isKeyDown(Key::S)) camera.position -= forward * (moveSpeed * deltaTime);
            if (input.isKeyDown(Key::D)) camera.position += right * (moveSpeed * deltaTime);
            if (input.isKeyDown(Key::A)) camera.position -= right * (moveSpeed * deltaTime);
            if (input.isKeyDown(Key::Space)) camera.position.y -= moveSpeed * deltaTime;
            if (input.isKeyDown(Key::LeftShift)) camera.position.y += moveSpeed * deltaTime;

            renderer.drawCube(Vect3(0.0f, 0.0f, -3.0f), Vect3(0.8f, 0.3f, 0.2f));
            renderer.drawCube(Vect3(3.0f, 0.0f, -3.0f), Vect3(0.2f, 0.8f, 0.3f), Mat4::identity(), Vect3(2.0f, 2.0f, 2.0f));
            renderer.drawCube(Vect3(-3.0f, 0.0f, -3.0f), Vect3(0.5f, 0.5f, 0.9f), Mat4::identity(), Vect3(2.0f, 0.2f, 2.0f));
            renderer.drawCube(Vect3(0.0f, 2.0f, -3.0f), Vect3(0.9f, 0.7f, 0.1f), Mat4::rotationY(0.785f));
            renderer.drawCube(Vect3(0.0f, -2.0f, -3.0f), Vect3(0.6f, 0.2f, 0.8f), Mat4::rotationZ(0.4f), Vect3(0.5f, 2.0f, 0.5f));
            renderer.drawCube(Vect3(4.0f, 0.0f, -6.0f), Vect3(1.0f, 0.5f, 0.0f), Mat4::rotationY(0.6f) * Mat4::rotationX(0.3f), Vect3(1.5f, 1.5f, 1.5f));
        });
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}
