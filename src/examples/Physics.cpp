#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "../Engine.hpp"
#include "../math/Vect3.hpp"
#include "../math/Mat4.hpp"
#include "../math/Quat.hpp"
#include "../rendering/Mesh.hpp"
#include "../scene/Scene.hpp"
#include "../physics/RigidBody.hpp"
#include "../physics/PhysicsWorld.hpp"

namespace {
    // Returns true for exactly one frame when `key` is pressed down.
    bool consumeKeyPress(Key key, Input& input, bool& wasDown) {
        bool down = input.isKeyDown(key);
        bool pressed = down && !wasDown;
        wasDown = down;
        return pressed;
    }

    Vect3 bodyColor(const RigidBody& b) {
        if (b.isStatic())
            return Vect3(0.50f, 0.53f, 0.56f);
        return b.shape == RigidBody::Shape::Sphere
            ? Vect3(0.30f, 0.62f, 1.0f)
            : Vect3(0.95f, 0.62f, 0.26f);
    }
}

int main() {
    try {
        Engine engine(1000, 700, "Physics");
        Font* font = engine.loadFont("assets/font.ttf", 40);
        engine.setSkyColor(Vect3(0.06f, 0.08f, 0.16f), Vect3(0.18f, 0.22f, 0.32f));

        Mesh cube = Mesh::createCube();          // unit box, half extents 0.5
        Mesh sphere = Mesh::createSphere(0.5f);  // radius 0.5 at scale 1

        Scene scene;
        PhysicsWorld world;
        world.setGravity(Vect3(0.0f, -15.0f, 0.0f));
        world.setSubsteps(4);

        // Bodies live in a vector of unique_ptr so the world's raw pointers
        // stay valid while we iterate them for drawing.
        std::vector<std::unique_ptr<RigidBody>> bodies;
        int towerCount = 0;
        int sphereCount = 0;

        auto rebuild = [&]() {
            world.clear();
            bodies.clear();
            scene.clear();

            auto addBody = [&](SceneNode& node, RigidBody::Shape shape) -> RigidBody& {
                bodies.push_back(std::make_unique<RigidBody>(node, shape));
                RigidBody& body = *bodies.back();
                world.add(body);
                return body;
            };

            // Arena floor.
            SceneNode& floorNode = scene.addNode("floor");
            floorNode.position.y = -0.5f;
            RigidBody& floor = addBody(floorNode, RigidBody::Shape::Box);
            floor.halfExtents = Vect3(12.0f, 0.5f, 12.0f);
            floor.setMass(0.0f);

            // Ramp that drops down toward the arena.
            SceneNode& rampNode = scene.addNode("ramp");
            rampNode.position = Vect3(0.0f, 1.8f, 5.0f);
            rampNode.rotation = Quat::axisAngle(Vect3(1.0f, 0.0f, 0.0f), -0.3f);
            RigidBody& ramp = addBody(rampNode, RigidBody::Shape::Box);
            ramp.halfExtents = Vect3(3.0f, 0.15f, 2.5f);
            ramp.setMass(0.0f);

            // Back wall to catch wayward boxes.
            SceneNode& wallNode = scene.addNode("wall");
            wallNode.position = Vect3(0.0f, 3.0f, -9.0f);
            RigidBody& wall = addBody(wallNode, RigidBody::Shape::Box);
            wall.halfExtents = Vect3(8.0f, 3.0f, 0.5f);
            wall.setMass(0.0f);

            // Box tower.
            towerCount = 0;
            for (int i = 0; i < 5; i++) {
                SceneNode& node = scene.addNode("tower" + std::to_string(i));
                node.position = Vect3(3.0f, 0.5f + i, 0.0f);
                node.scale = Vect3(0.85f, 0.85f, 0.85f);
                RigidBody& box = addBody(node, RigidBody::Shape::Box);
                box.halfExtents = Vect3(0.5f, 0.5f, 0.5f);
                box.setRestitution(0.05f);
                box.setFriction(0.6f);
                towerCount++;
            }

            // A couple of loose boxes that will tumble when hit.
            {
                SceneNode& node = scene.addNode("tumbler");
                node.position = Vect3(1.0f, 6.0f, 2.0f);
                node.rotation = Quat::axisAngle(Vect3(0.0f, 0.0f, 1.0f), 0.6f);
                node.scale = Vect3(0.9f, 0.7f, 0.9f);
                RigidBody& box = addBody(node, RigidBody::Shape::Box);
                box.halfExtents = Vect3(0.5f, 0.4f, 0.5f);
                box.setRestitution(0.2f);
            }

            // Falling spheres.
            sphereCount = 0;
            const float positions[][3] = {
                { -2.5f, 8.0f, -2.0f },
                { -1.2f, 10.0f, 0.5f },
                { 2.0f, 9.0f, 1.0f },
                { 0.5f, 12.0f, -2.5f },
            };
            for (int i = 0; i < 4; i++) {
                SceneNode& node = scene.addNode("sphere" + std::to_string(i));
                node.position = Vect3(positions[i][0], positions[i][1], positions[i][2]);
                node.scale = Vect3(0.8f, 0.8f, 0.8f);
                RigidBody& ball = addBody(node, RigidBody::Shape::Sphere);
                ball.radius = 0.4f;
                ball.setRestitution(0.5f);
                sphereCount++;
            }
        };
        rebuild();

        bool paused = false;
        bool wasP = false;
        bool wasR = false;

        const float moveSpeed = 6.0f;
        const float mouseSensitivity = 0.0025f;
        const float maxPitch = 1.5f;

        engine.runFixed(1.0f / 60.0f,
            [&](float dt, Renderer&, Camera& camera, Input& input) {
                static bool cameraInitialized = false;
                if (!cameraInitialized) {
                    camera.position = Vect3(0.0f, 7.0f, 16.0f);
                    camera.pitch = -0.35f;
                    cameraInitialized = true;
                }

                if (consumeKeyPress(Key::P, input, wasP)) paused = !paused;
                if (consumeKeyPress(Key::R, input, wasR)) rebuild();

                camera.yaw -= input.getMouseDeltaX() * mouseSensitivity;
                camera.pitch -= input.getMouseDeltaY() * mouseSensitivity;
                camera.pitch = std::clamp(camera.pitch, -maxPitch, maxPitch);
                if (!paused) {
                    Vect3 forward = camera.getForward();
                    Vect3 right = camera.getRight();
                    if (input.isKeyDown(Key::W)) camera.position += forward * (moveSpeed * dt);
                    if (input.isKeyDown(Key::S)) camera.position -= forward * (moveSpeed * dt);
                    if (input.isKeyDown(Key::D)) camera.position += right * (moveSpeed * dt);
                    if (input.isKeyDown(Key::A)) camera.position -= right * (moveSpeed * dt);
                    if (input.isKeyDown(Key::Space)) camera.position.y += moveSpeed * dt;
                    if (input.isKeyDown(Key::LeftShift)) camera.position.y -= moveSpeed * dt;

                    world.step(dt);
                }
            },
            [&](float, Renderer& renderer, Camera&, Input&) {
                renderer.lighting.lights.clear();

                Light key;
                key.type = Light::Type::Directional;
                key.direction = Vect3(-0.4f, -0.9f, -0.25f).normalize();
                key.color = Vect3(1.0f, 0.95f, 0.85f);
                key.intensity = 0.75f;
                renderer.lighting.lights.push_back(key);

                Light fill;
                fill.type = Light::Type::Directional;
                fill.direction = Vect3(0.5f, 0.3f, 0.7f).normalize();
                fill.color = Vect3(0.45f, 0.55f, 0.8f);
                fill.intensity = 0.25f;
                renderer.lighting.lights.push_back(fill);

                Light rampLight;
                rampLight.type = Light::Type::Point;
                rampLight.position = Vect3(0.0f, 2.5f, 5.0f);
                rampLight.color = Vect3(1.0f, 0.8f, 0.55f);
                rampLight.intensity = 2.5f;
                rampLight.attenuation = Vect3(0.0f, 0.06f, 0.008f);
                renderer.lighting.lights.push_back(rampLight);

                for (RigidBody* body : world.getBodies()) {
                    const Mesh& mesh = body->shape == RigidBody::Shape::Sphere ? sphere : cube;
                    renderer.drawMesh(mesh, body->node->getWorldMatrix(), bodyColor(*body));
                }

                renderer.drawTextScreen(*font,
                    "Physics: impulse solver with sphere / OBB (SAT) collisions",
                    12.0f, 12.0f, 18.0f, Vect3(1.0f, 1.0f, 1.0f));
                renderer.drawTextScreen(*font,
                    "WASD+mouse fly | R: reset | P: " + std::string(paused ? "resume" : "pause") + " | Esc frees cursor",
                    12.0f, 36.0f, 13.0f, Vect3(0.72f, 0.76f, 0.82f));
                renderer.drawTextScreen(*font,
                    "bodies " + std::to_string(world.getBodyCount()) +
                    " (tower " + std::to_string(towerCount) +
                    ", spheres " + std::to_string(sphereCount) + ") | contacts " +
                    std::to_string(world.getContactCount()),
                    12.0f, 56.0f, 13.0f, Vect3(0.9f, 0.95f, 0.6f));
            });

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }
}