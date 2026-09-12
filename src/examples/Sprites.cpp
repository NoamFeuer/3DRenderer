#include <iostream>
#include "../Engine.hpp"
#include "../math/Vect3.hpp"
#include "../math/Mat4.hpp"

int main() {
    try {
        Engine engine(800, 600, "2D Sprites");

        Vect3 ballPos(-1.5f, 1.0f, 0.0f);
        Vect3 ballVel(3.0f, -5.0f, 0.0f);
        float ballRotation = 0.0f;
        const float boundsTop = 3.5f, boundsBottom = -3.5f, boundsLeft = -5.0f, boundsRight = 5.0f;

        // Visual state (interpolated in render).
        Vect3 renderBallPos = ballPos;

        int checkerTex = engine.loadTexture("assets/checker.png");

        engine.runFixed(
            1.0f / 60.0f,
            [&](float deltaTime, Renderer& renderer, Camera& camera, Input& input) {
                (void)renderer;
                // 2D setup: orthographic view, camera pulled back along +Z so
                // the XY plane at z=0 sits in front of it. orthoHalfHeight is
                // the vertical half-extent of the visible world in units.
                camera.type = Camera::Type::Orthographic;
                camera.position.z = 5.0f;
                camera.orthoHalfHeight = 4.0f;

                // Scroll the 2D camera with the arrow keys.
                if (input.isKeyDown(Key::Up))    camera.position.y += 5.0f * deltaTime;
                if (input.isKeyDown(Key::Down))  camera.position.y -= 5.0f * deltaTime;
                if (input.isKeyDown(Key::Left))  camera.position.x -= 5.0f * deltaTime;
                if (input.isKeyDown(Key::Right)) camera.position.x += 5.0f * deltaTime;

                // Deterministic ball physics.
                ballPos += ballVel * deltaTime;
                if (ballPos.y > boundsTop)    { ballPos.y = boundsTop;    ballVel.y = -ballVel.y; }
                if (ballPos.y < boundsBottom) { ballPos.y = boundsBottom; ballVel.y = -ballVel.y; }
                if (ballPos.x > boundsRight)  { ballPos.x = boundsRight;  ballVel.x = -ballVel.x; }
                if (ballPos.x < boundsLeft)   { ballPos.x = boundsLeft;   ballVel.x = -ballVel.x; }
                ballRotation += 2.0f * deltaTime;
            },
            [&](float alpha, Renderer& renderer, Camera& camera, Input& input) {
                (void)input;
                (void)camera;
                // Interpolate the ball between simulation steps.
                renderBallPos = ballPos + ballVel * alpha * (1.0f / 60.0f);

                // Background band drawn first (painter's order resolves ties).
                renderer.drawRectangle(
                    Vect3(-6.0f, boundsBottom - 0.2f, 0.0f),
                    Vect3( 6.0f, boundsBottom - 0.2f, 0.0f),
                    Vect3( 6.0f, boundsBottom + 0.3f, 0.0f),
                    Vect3(-6.0f, boundsBottom + 0.3f, 0.0f),
                    Vect3(0.2f, 0.2f, 0.25f));

                // A spinning sprite with per-frame Z rotation.
                renderer.drawSprite(renderBallPos, Vect3(1.2f, 1.2f, 1.0f), checkerTex,
                                    Vect3(1.0f, 1.0f, 1.0f), ballRotation);

                // Static decorative sprites.
                renderer.drawSprite(Vect3(-4.0f, 2.5f, 0.0f), Vect3(1.0f, 1.0f, 1.0f), checkerTex, Vect3(0.6f, 0.9f, 0.6f), 0.2f);
                renderer.drawSprite(Vect3(4.0f, -2.5f, 0.0f), Vect3(2.0f, 1.0f, 1.0f), checkerTex, Vect3(0.9f, 0.6f, 0.6f), -0.3f);

                // Solid-color 2D shapes mix in freely.
                renderer.drawCircle(Vect3(3.0f, 2.5f, 0.0f), 0.6f, Vect3(0.8f, 0.8f, 0.2f), 24);
            });
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}