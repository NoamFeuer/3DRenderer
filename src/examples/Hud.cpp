#include <iostream>
#include <string>
#include "../Engine.hpp"
#include "../math/Vect3.hpp"
#include "../math/Mat4.hpp"

// Screen-space text: HUD overlays drawn with Renderer::drawTextScreen(). The
// world scene (rotating cubes with world labels) stays in the background while
// the FPS counter, title and hint stay fixed on screen regardless of the
// camera. Cursor: Esc to release, left-click to re-capture; WASD + mouse to fly.
int main() {
    try {
        Engine engine(800, 600, "HUD");

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

            // ---- World scene: opaque cubes, then a world-space label. ----
            renderer.drawCube(Vect3(0.0f, 0.0f, -3.0f), Vect3(0.8f, 0.3f, 0.2f),
                              Mat4::rotationY(elapsed * 0.8f));
            renderer.drawCube(Vect3(3.0f, 0.0f, -3.0f), Vect3(0.2f, 0.8f, 0.3f),
                              Mat4::rotationY(-elapsed * 0.5f), Vect3(1.5f, 1.5f, 1.5f));
            renderer.drawCube(Vect3(-3.0f, 0.0f, -3.0f), Vect3(0.5f, 0.5f, 0.9f),
                              Mat4::identity(), Vect3(2.0f, 0.4f, 2.0f));
            renderer.drawText(*font, "world", Vect3(0.8f, 1.6f, -3.0f), 0.5f, Vect3(1.0f, 0.8f, 0.7f));

            // ---- Screen-space HUD (pixel coords, top-left origin). ----
            int height = engine.getWindowHeight();

            std::string frameRate = "FPS " + std::to_string(static_cast<int>(1.0f / (deltaTime > 0.0f ? deltaTime : 1.0f)));
            renderer.drawTextScreen(*font, frameRate, 16.0f, 16.0f, 28.0f, Vect3(0.2f, 1.0f, 0.4f));

            renderer.drawTextScreen(*font, "3D Renderer", 16.0f, 48.0f, 24.0f, Vect3(1.0f, 1.0f, 1.0f));

            renderer.drawTextScreen(*font, "WASD + mouse to look around", 16.0f,
                                    static_cast<float>(height) - 44.0f, 20.0f, Vect3(0.7f, 0.7f, 0.7f));
        });
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}