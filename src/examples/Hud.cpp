#include <iostream>
#include <string>
#include "../Engine.hpp"
#include "../math/Vect3.hpp"
#include "../math/Mat4.hpp"

int main() {
    try {
        Engine engine(800, 600, "HUD");

        Font* font = engine.loadFont("assets/font.ttf", 64);
        int checkerTex = engine.loadTexture("assets/checker.png");

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

            renderer.drawCube(Vect3(0.0f, 0.0f, -3.0f), Vect3(0.8f, 0.3f, 0.2f), Mat4::rotationY(elapsed * 0.8f));
            renderer.drawCube(Vect3(3.0f, 0.0f, -3.0f), Vect3(0.2f, 0.8f, 0.3f), Mat4::rotationY(-elapsed * 0.5f), Vect3(1.5f, 1.5f, 1.5f));
            renderer.drawCube(Vect3(-3.0f, 0.0f, -3.0f), Vect3(0.5f, 0.5f, 0.9f), Mat4::identity(), Vect3(2.0f, 0.4f, 2.0f));
            renderer.drawText(*font, "world", Vect3(0.8f, 1.6f, -3.0f), 0.5f, Vect3(1.0f, 0.8f, 0.7f));

            float width = static_cast<float>(engine.getWindowWidth());
            float height = static_cast<float>(engine.getWindowHeight());

            renderer.drawPanelScreen(8.0f, 8.0f, 260.0f, 88.0f, Vect3(0.06f, 0.08f, 0.12f));
            std::string frameRate = "FPS " + std::to_string(static_cast<int>(1.0f / (deltaTime > 0.0f ? deltaTime : 1.0f)));
            renderer.drawTextScreen(*font, frameRate, 16.0f, 16.0f, 28.0f, Vect3(0.2f, 1.0f, 0.4f));
            renderer.drawTextScreen(*font, "3D Renderer", 16.0f, 48.0f, 24.0f, Vect3(1.0f, 1.0f, 1.0f));
            renderer.drawTextScreen(*font, "HUD panels demo", 16.0f, 76.0f, 16.0f, Vect3(0.6f, 0.65f, 0.7f));

            renderer.drawPanelScreen(width - 68.0f, 8.0f, 60.0f, 45.0f, checkerTex);

            float health = 0.35f + 0.3f * (0.5f + 0.5f * std::sin(elapsed * 1.5f));
            float barX = 16.0f;
            float barY = height - 74.0f;
            float barW = 180.0f;
            renderer.drawPanelScreen(barX, barY, barW, 16.0f, Vect3(0.15f, 0.15f, 0.18f));
            renderer.drawPanelScreen(barX, barY, barW * health, 16.0f, Vect3(0.25f, 0.9f, 0.4f));
            renderer.drawTextScreen(*font, "WASD + mouse to look around", 16.0f, static_cast<float>(height) - 44.0f, 20.0f, Vect3(0.7f, 0.7f, 0.7f));
        });
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}