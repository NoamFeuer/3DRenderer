#pragma once

#include "../math/Vect3.hpp"
#include "../math/Quat.hpp"
#include "../math/Mat4.hpp"
#include "../scene/SceneNode.hpp"

// A rigid body that drives the transform of a SceneNode. The node's local
// position and rotation are moved by integration and collision resolution, so
// bodies respect the scene-graph hierarchy (a body attached to a child moves
// relative to its parent). Colliders are centered on the node origin.
class RigidBody {
public:
    enum class Shape { Sphere, Box };

    RigidBody(SceneNode& node, Shape shape = Shape::Sphere);

    SceneNode* node = nullptr;
    Shape shape = Shape::Sphere;

    // Local-space shape parameters.
    float radius = 0.5f;
    Vect3 halfExtents{ 0.5f, 0.5f, 0.5f };

    float mass = 1.0f;
    float invMass = 1.0f;
    float restitution = 0.25f;
    float friction = 0.4f;
    float linearDamping = 0.05f;
    float angularDamping = 0.1f;
    float gravityScale = 1.0f;

    Vect3 velocity;
    Vect3 angularVelocity; // rad/s, world space

    void setMass(float mass);
    void setRestitution(float r) { restitution = r; }
    void setFriction(float f) { friction = f; }
    void setGravityScale(float g) { gravityScale = g; }

    bool isStatic() const { return invMass == 0.0f; }

    // World-space shape. Uniform-scale assumption only for spheres; boxes go
    // through the full node transform, so rotated/scaled boxes work.
    Vect3 getWorldCenter() const;
    float getWorldRadius() const;
    void getWorldObb(Vect3& center, Vect3 axes[3], Vect3& halfExtentsWorld) const;

    // World-space inverse inertia tensor (a pure rotation matrix product).
    Mat4 invInertiaWorld;

    // j is an impulse applied at contactPoint (world space). Updates linear
    // and angular velocity.
    void applyImpulse(const Vect3& j, const Vect3& contactPoint);

    // Refreshes invInertiaWorld from the current node rotation and shape.
    void updateInverseInertia();

private:
    void computeWorldInertia(float& invIx, float& invIy, float& invIz);
};