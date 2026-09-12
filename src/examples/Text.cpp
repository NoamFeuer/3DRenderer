#include <iostream>
#include <string>
#include "../Engine.hpp"
#include "../math/Vect3.hpp"
#include "../math/Mat4.hpp"

// Demonstrates Renderer::drawText(): world-space labels above 3D objects using
// a font atlas, drawn in the alpha-blended pass after the opaque geometry.
// Cursor: Esc to release, left-click to re-capture; WASD + mouse to fly.
int main() {
    try {
        Engine engine(800, 600, "Text Rendering");

        Font* font = engine.loadFont("assets/font.ttf", 64);

        const float moveSpeed = 3.0f;
        const float mouseSensitivity = 0.0025f;
        float elapsed = 0.0f;

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

            elapsed += deltaTime;

            // Opaque world first, then blended text.
            renderer.drawCube(Vect3(0.0f, 0.0f, -3.0f), Vect3(0.8f, 0.3f, 0.2f),
                              Mat4::rotationY(elapsed * 0.8f));
            renderer.drawCube(Vect3(3.0f, 0.0f, -3.0f), Vect3(0.2f, 0.8f, 0.3f),
                              Mat4::rotationY(-elapsed * 0.5f), Vect3(1.5f, 1.5f, 1.5f));
            renderer.drawCube(Vect3(-3.0f, 0.0f, -3.0f), Vect3(0.5f, 0.5f, 0.9f),
                              Mat4::identity(), Vect3(2.0f, 0.4f, 2.0f));

            // Floating labels above each cube (drawn in world space).
            renderer.drawText(*font, "alpha", Vect3(-0.4f, 1.2f, -3.0f), 0.5f, Vect3(1.0f, 0.8f, 0.7f));
            renderer.drawText(*font, "beta", Vect3(2.6f, 1.2f, -3.0f), 0.5f, Vect3(0.7f, 1.0f, 0.8f));
            renderer.drawText(*font, "gamma", Vect3(-3.4f, 1.2f, -3.0f), 0.5f, Vect3(0.8f, 0.8f, 1.0f));

            // A constantly-updating status line.
            std::string status = "elapsed " + std::to_string(static_cast<int>(elapsed)) + "s  |  fps " +
                                 std::to_string(static_cast<int>(1.0f / (deltaTime > 0.0f ? deltaTime : 1.0f)));
            renderer.drawText(*font, status, Vect3(-6.0f, -1.8f, -3.0f), 0.4f, Vect3(0.9f, 0.9f, 0.9f));
        });
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}