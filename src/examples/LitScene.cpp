#include <iostream>
#include <string>
#include "../Engine.hpp"
#include "../math/Vect3.hpp"
#include "../math/Mat4.hpp"
#include "../rendering/Mesh.hpp"

// Demonstrates the lighting/material milestone: directional, point and spot
// lights, emissive/glossy materials, an OBJ mesh (pyramid), a mesh ground
// plane, MSAA, and the procedural sky gradient.
int main() {
    try {
        Engine engine(1000, 700, "Lighting");
        Font* font = engine.loadFont("assets/font.ttf", 40);

        engine.setSkyColor(Vect3(0.13f, 0.18f, 0.38f), Vect3(0.42f, 0.47f, 0.55f));

        Mesh floor = Mesh::createPlane(18.0f, 18.0f, 1);
        Mesh pyramid = Mesh::loadObj("assets/pyramid.obj");

        Material glossy;
        glossy.specular = Vect3(0.6f, 0.6f, 0.6f);
        glossy.shininess = 96.0f;

        Material matte;
        matte.specular = Vect3(0.01f, 0.01f, 0.01f);
        matte.shininess = 8.0f;

        Material glowing;
        glowing.emission = Vect3(1.0f, 0.55f, 0.15f);
        glowing.specular = Vect3(0.0f, 0.0f, 0.0f);

        const float moveSpeed = 4.0f;
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

            // ---- Lights (replace the default headlight) ----
            renderer.lighting.lights.clear();

            Light key;
            key.type = Light::Type::Directional;
            key.direction = Vect3(-0.4f, -0.9f, -0.2f).normalize();
            key.color = Vect3(1.0f, 0.95f, 0.85f);
            key.intensity = 0.9f;
            renderer.lighting.lights.push_back(key);

            Light orb;
            orb.type = Light::Type::Point;
            orb.position = Vect3(std::cos(elapsed) * 4.0f, 2.2f, std::sin(elapsed) * 4.0f);
            orb.color = Vect3(1.0f, 0.4f, 0.3f);
            orb.intensity = 2.5f;
            orb.attenuation = Vect3(0.2f, 0.35f, 0.04f);
            renderer.lighting.lights.push_back(orb);

            Light spot;
            spot.type = Light::Type::Spot;
            spot.position = Vect3(-4.0f, 3.0f, 2.0f);
            spot.direction = Vect3(4.0f, -3.0f, -2.0f).normalize();
            spot.color = Vect3(0.5f, 0.8f, 1.0f);
            spot.intensity = 3.0f;
            spot.spotInnerCos = std::cos(0.25f);
            spot.spotOuterCos = std::cos(0.5f);
            renderer.lighting.lights.push_back(spot);

            // ---- Scene ----
            renderer.drawMesh(floor, Vect3(0.0f, -0.6f, 0.0f), Vect3(0.5f, 0.5f, 0.55f), matte);
            renderer.drawMesh(pyramid, Vect3(0.0f, 0.0f, -1.5f), Vect3(0.9f, 0.9f, 0.8f),
                              glossy, Mat4::rotationY(elapsed * 0.5f), Vect3(1.4f, 1.4f, 1.4f));
            renderer.drawSphere(Vect3(2.2f, 0.4f, 1.0f), 0.4f, Vect3(0.3f, 0.7f, 0.9f), glossy, 32, 16);
            renderer.drawCube(Vect3(-2.2f, 0.4f, -1.5f), Vect3(0.85f, 0.4f, 0.25f),
                              Mat4::rotationY(elapsed * 0.7f), Vect3(0.8f, 0.8f, 0.8f), matte);
            renderer.drawCube(Vect3(0.0f, 0.55f, 2.5f), Vect3(1.0f, 0.75f, 0.2f),
                              Mat4::identity(), Vect3(0.7f, 0.7f, 0.7f), glowing);

            // Light markers (unlit spheres at the light positions).
            Material marker;
            marker.unlit = true;
            renderer.drawSphere(orb.position, 0.12f, orb.color, marker, 16, 8);
            renderer.drawSphere(spot.position, 0.12f, spot.color, marker, 16, 8);

            renderer.drawTextScreen(*font, "Directional + orbiting point + spot light", 12.0f, 12.0f, 20.0f, Vect3(1.0f, 1.0f, 1.0f));
            renderer.drawTextScreen(*font, "WASD + mouse to look around  |  sky + MSAA enabled", 12.0f, 40.0f, 14.0f, Vect3(0.7f, 0.75f, 0.8f));
        });
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}