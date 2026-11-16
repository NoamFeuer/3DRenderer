#include <algorithm>
#include <cmath>
#include <iostream>

#include "../Engine.hpp"
#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"
#include "../rendering/Mesh.hpp"

int main() {
    try {
        Engine engine(1000, 700, "Model");

        const char* modelPath = "assets/duck.glb";

        Mesh model = Mesh::loadModel(modelPath);
        std::cout << "Loaded " << modelPath << ": " << model.vertices.size() << " vertices, " << model.indices.size() / 3 << " triangles" << std::endl;

        uint32_t meshId = engine.uploadStaticMesh(model, Vect3(1.0f, 1.0f, 1.0f));

        Material material;
        material.specular = Vect3(0.3f, 0.3f, 0.3f);
        material.shininess = 32.0f;

        const float moveSpeed = 3.0f;
        bool framed = false;

        engine.run([&](float deltaTime, Renderer& renderer, Camera& camera, Input& input) {
            if (!framed) {
                camera.position = Vect3(0.0f, 1.2f, 3.5f);
                framed = true;
            }

            camera.yaw -= input.getMouseDeltaX() * 0.0025f;
            camera.pitch -= input.getMouseDeltaY() * 0.0025f;
            camera.pitch = std::clamp(camera.pitch, -1.4f, 1.4f);

            const Vect3 forward = camera.getForward();
            const Vect3 right = camera.getRight();

            if (input.isKeyDown(Key::W)) camera.position += forward * (moveSpeed * deltaTime);
            if (input.isKeyDown(Key::S)) camera.position -= forward * (moveSpeed * deltaTime);
            if (input.isKeyDown(Key::D)) camera.position += right * (moveSpeed * deltaTime);
            if (input.isKeyDown(Key::A)) camera.position -= right * (moveSpeed * deltaTime);
            if (input.isKeyDown(Key::Space)) camera.position.y += moveSpeed * deltaTime;
            if (input.isKeyDown(Key::LeftShift)) camera.position.y -= moveSpeed * deltaTime;

            renderer.lighting.lights.clear();

            Light key;
            key.type = Light::Type::Directional;
            key.direction = Vect3(-0.4f, -0.9f, -0.3f).normalize();
            key.color = Vect3(1.0f, 0.96f, 0.9f);
            key.intensity = 1.0f;
            renderer.lighting.lights.push_back(key);

            Light fill;
            fill.type = Light::Type::Directional;
            fill.direction = Vect3(0.6f, -0.2f, 0.5f).normalize();
            fill.color = Vect3(0.5f, 0.6f, 0.8f);
            fill.intensity = 0.35f;
            renderer.lighting.lights.push_back(fill);

            renderer.drawStaticMesh(meshId, Vect3(0.0f, 0.0f, 0.0f), Mat4::identity(), Vect3(1.0f, 1.0f, 1.0f), material);
        });
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return -1;
    }

    return 0;
}