#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <limits>

#include "../Engine.hpp"
#include "../math/Vect3.hpp"
#include "../math/Mat4.hpp"
#include "../math/Quat.hpp"
#include "../math/Aabb.hpp"
#include "../math/Ray.hpp"
#include "../math/Frustum.hpp"
#include "../rendering/Mesh.hpp"
#include "../scene/Scene.hpp"

namespace {
    // Returns true for exactly one frame when `key` is pressed down.
    bool consumeKeyPress(Key key, Input& input, bool& wasDown) {
        bool down = input.isKeyDown(key);
        bool pressed = down && !wasDown;
        wasDown = down;
        return pressed;
    }
}

struct Body {
    SceneNode* node = nullptr;
    float orbitRadius = 0.0f;
    float orbitSpeed = 0.0f;
    float spinSpeed = 0.0f;
    float angle = 0.0f;
    float spinAngle = 0.0f;
    Quat tilt;
    Vect3 color{ 0.7f, 0.7f, 0.7f };
    Material material;
    bool wobble = false;
    Aabb localBounds;

    void update(float dt) {
        angle += orbitSpeed * dt;
        spinAngle += spinSpeed * dt;

        // The node's local transform is rebuilt each frame. When a body has a
        // parent (the moon), the parent's transform is composed in when asking
        // for the world matrix, so the whole orbit follows the planet.
        node->position = Vect3(std::cos(angle) * orbitRadius, 0.0f, std::sin(angle) * orbitRadius);
        node->rotation = spinAngle == 0.0f ? tilt : Quat::axisAngle(Vect3(0.0f, 1.0f, 0.0f), spinAngle) * tilt;

        if (wobble) {
            // Show off slerp: rock the body between two tilts over time.
            Quat a = node->rotation;
            Quat b = node->rotation * Quat::axisAngle(Vect3(0.0f, 0.0f, 1.0f), 1.1f);
            node->rotation = Quat::slerp(a, b, 0.5f + 0.5f * std::sin(spinAngle * 0.6f));
        }
    }
};

int main() {
    try {
        Engine engine(1000, 700, "Scene Graph");
        Font* font = engine.loadFont("assets/font.ttf", 40);
        engine.setSkyColor(Vect3(0.07f, 0.09f, 0.19f), Vect3(0.2f, 0.24f, 0.34f));

        Mesh sphere = Mesh::createSphere(0.5f, 24, 12);
        const Vect3 half(0.5f, 0.5f, 0.5f);

        // ---- Build the scene graph ----
        Scene scene;

        const auto addSphere = [&](SceneNode& node, float worldRadius, const Vect3& color, const Material& mat) -> Body {
            Body b;
            b.node = &node;
            b.color = color;
            b.material = mat;
            float s = worldRadius * 2.0f;   // mesh base radius is 0.5
            node.scale = Vect3(s, s, s);
            b.localBounds = Aabb::fromCenterHalfSize(Vect3(0.0f, 0.0f, 0.0f), half);
            return b;
        };

        Body sun = addSphere(scene.addNode("sun"), 1.6f, Vect3(1.0f, 0.62f, 0.18f), Material{});
        sun.material.unlit = true;
        sun.material.emission = Vect3(1.0f, 0.5f, 0.12f);

        Body mercury = addSphere(scene.addNode("mercury"), 0.30f, Vect3(0.85f, 0.55f, 0.35f), Material{});
        mercury.orbitRadius = 4.0f;
        mercury.orbitSpeed = 1.3f;
        mercury.spinSpeed = 2.0f;
        mercury.tilt = Quat::axisAngle(Vect3(1, 0, 0), 0.4f);
        mercury.material.specular = Vect3(0.4f, 0.4f, 0.4f);
        mercury.material.shininess = 32.0f;

        Body earth = addSphere(scene.addNode("earth"), 0.45f, Vect3(0.3f, 0.6f, 0.9f), Material{});
        earth.orbitRadius = 7.0f;
        earth.orbitSpeed = 0.9f;
        earth.spinSpeed = 1.2f;
        earth.tilt = Quat::axisAngle(Vect3(0, 0, 1), 0.35f);
        earth.material.specular = Vect3(0.4f, 0.4f, 0.4f);
        earth.material.shininess = 48.0f;

        Body giant = addSphere(scene.addNode("giant"), 0.80f, Vect3(0.8f, 0.55f, 0.9f), Material{});
        giant.orbitRadius = 10.5f;
        giant.orbitSpeed = 0.5f;
        giant.spinSpeed = 0.7f;
        giant.tilt = Quat::axisAngle(Vect3(0, 0, 1), -0.35f);
        giant.node->scale = Vect3(1.6f, 1.15f, 1.6f);   // slightly squashed
        giant.wobble = true;
        giant.material.specular = Vect3(0.6f, 0.6f, 0.6f);
        giant.material.shininess = 64.0f;

        // The moon is a child of earth: its position is relative to earth, so
        // its orbit follows the planet through the scene graph.
        Body moon = addSphere(scene.addNode("moon", earth.node), 0.15f, Vect3(0.7f, 0.72f, 0.78f), Material{});
        moon.orbitRadius = 1.25f;
        moon.orbitSpeed = 2.8f;
        moon.spinSpeed = 1.0f;

        std::vector<Body*> drawables = { &sun, &mercury, &earth, &giant, &moon };

        // ---- Camera + controls ----
        const float moveSpeed = 5.0f;
        const float mouseSensitivity = 0.0025f;
        const float maxPitch = 1.5f;

        bool cullingEnabled = true;
        bool paused = false;
        bool wasE = false;
        bool wasQ = false;
        SceneNode* selected = nullptr;
        Mat4 inverseViewProjection;

        engine.run([&](float deltaTime, Renderer& renderer, Camera& camera, Input& input) {
            static bool cameraInitialized = false;
            if (!cameraInitialized) {
                camera.position = Vect3(0.0f, 6.0f, 14.0f);
                camera.pitch = -0.405f;
                cameraInitialized = true;
            }

            if (consumeKeyPress(Key::E, input, wasE)) cullingEnabled = !cullingEnabled;
            if (consumeKeyPress(Key::Q, input, wasQ)) paused = !paused;

            camera.yaw -= input.getMouseDeltaX() * mouseSensitivity;
            camera.pitch -= input.getMouseDeltaY() * mouseSensitivity;
            if (camera.pitch > maxPitch) camera.pitch = maxPitch;
            if (camera.pitch < -maxPitch) camera.pitch = -maxPitch;

            if (!paused) {
                Vect3 forward = camera.getForward();
                Vect3 right = camera.getRight();
                if (input.isKeyDown(Key::W)) camera.position += forward * (moveSpeed * deltaTime);
                if (input.isKeyDown(Key::S)) camera.position -= forward * (moveSpeed * deltaTime);
                if (input.isKeyDown(Key::D)) camera.position += right * (moveSpeed * deltaTime);
                if (input.isKeyDown(Key::A)) camera.position -= right * (moveSpeed * deltaTime);
                if (input.isKeyDown(Key::Space)) camera.position.y += moveSpeed * deltaTime;
                if (input.isKeyDown(Key::LeftShift)) camera.position.y -= moveSpeed * deltaTime;

                for (Body* b : drawables)
                    b->update(deltaTime);
            }

            // ---- Lighting ----
            renderer.lighting.lights.clear();

            Light key;
            key.type = Light::Type::Directional;
            key.direction = Vect3(-0.4f, -0.9f, -0.2f).normalize();
            key.color = Vect3(1.0f, 0.95f, 0.85f);
            key.intensity = 0.7f;
            renderer.lighting.lights.push_back(key);

            Light sunLight;
            sunLight.type = Light::Type::Point;
            sunLight.position = sun.node->getWorldPosition();
            sunLight.color = Vect3(1.0f, 0.7f, 0.4f);
            sunLight.intensity = 3.0f;
            sunLight.attenuation = Vect3(0.1f, 0.04f, 0.01f);
            renderer.lighting.lights.push_back(sunLight);

            // ---- Frustum culling ----
            float width = static_cast<float>(engine.getWindowWidth());
            float height = static_cast<float>(engine.getWindowHeight());
            Mat4 viewProjection = camera.getProjectionMatrix(width / height) * camera.getViewMatrix();
            inverseViewProjection = viewProjection.inverse();
            Frustum frustum = Frustum::fromViewProjection(viewProjection);

            int drawn = 0;
            for (Body* b : drawables) {
                Aabb worldBounds = b->localBounds.transformed(b->node->getWorldMatrix());

                if (cullingEnabled && !frustum.intersects(worldBounds))
                    continue;
                drawn++;

                Vect3 color = b->color;
                if (b->node == selected && b->node != sun.node)
                    color = Vect3(1.0f, 0.95f, 0.9f);   // highlight the picked body

                renderer.drawMesh(sphere, b->node->getWorldMatrix(), color, b->material);
            }

            // ---- Picking: cast a ray through the cursor ----
            if (!input.isCursorCaptured() && input.isMouseButtonPressed()) {
                float ndcX = 2.0f * input.getMouseX() / width - 1.0f;
                float ndcY = 1.0f - 2.0f * input.getMouseY() / height;

                Ray ray = Ray::fromNdc(inverseViewProjection, ndcX, ndcY);
                float bestT = std::numeric_limits<float>::infinity();
                SceneNode* best = nullptr;

                for (Body* b : drawables) {
                    Aabb worldBounds = b->localBounds.transformed(b->node->getWorldMatrix());
                    float t0 = 0.0f, t1 = 0.0f;
                    if (ray.intersectsAabb(worldBounds, t0, t1) && t1 > 0.0f) {
                        float hit = std::max(t0, 0.0f);
                        if (hit < bestT) {
                            bestT = hit;
                            best = b->node;
                        }
                    }
                }
                selected = best;
            }

            // ---- HUD ----
            renderer.drawTextScreen(*font, "Scene graph: hierarchy + quaternions + frustum culling + ray picking", 12.0f, 12.0f, 18.0f, Vect3(1.0f, 1.0f, 1.0f));
            renderer.drawTextScreen(*font,
                "WASD+mouse fly | E: culling " + std::string(cullingEnabled ? "ON" : "OFF") +
                " | Q: " + std::string(paused ? "resume" : "pause") +
                " | Esc frees cursor, click picks a body", 12.0f, 36.0f, 13.0f, Vect3(0.72f, 0.76f, 0.82f));

            std::string selectedName = selected ? selected->name : "(none)";
            renderer.drawTextScreen(*font,
                "bodies " + std::to_string(drawables.size()) +
                " | drawn " + std::to_string(drawn) +
                " | selected " + selectedName, 12.0f, 56.0f, 13.0f, Vect3(0.9f, 0.95f, 0.6f));
        });

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }
}