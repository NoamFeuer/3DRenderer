#pragma once

#include <vector>

#include "../math/Vect3.hpp"
#include "RigidBody.hpp"

struct Contact {
    RigidBody* a;
    RigidBody* b;
    Vect3 normal; // unit, points from a toward b
    float penetration;
    Vect3 point; // world-space contact point
};

// A simple impulse-based rigid body solver. Supports sphere and box (OBB)
// colliders. Bodies are driven by gravity and velocity each step, collisions
// are detected between every dynamic pair, and contacts are resolved with an
// impulse (restitution + Coulomb friction) plus positional correction, so
// stacks of boxes and bouncing balls behave stably.
class PhysicsWorld {
public:
    void add(RigidBody& body);
    void remove(RigidBody& body);
    void clear();

    void setGravity(const Vect3& g) { gravity = g; }
    Vect3 getGravity() const { return gravity; }

    void setSubsteps(int count) { substeps = count < 1 ? 1 : count; }
    int getSubsteps() const { return substeps; }

    int getBodyCount() const { return static_cast<int>(bodies.size()); }
    const std::vector<RigidBody*>& getBodies() const { return bodies; }
    // Contact count from the last solved substep (diagnostics/HUD).
    int getContactCount() const { return static_cast<int>(contacts.size()); }

    void step(float dt);

private:
    Vect3 gravity{ 0.0f, -9.81f, 0.0f };
    std::vector<RigidBody*> bodies;
    std::vector<Contact> contacts;
    int substeps = 4;

    void integrate(RigidBody& body, float dt);
    void detectCollisions();

    bool collide(RigidBody& a, RigidBody& b, Contact& out) const;
    bool collideSphereSphere(RigidBody& a, RigidBody& b, Contact& out) const;
    bool collideSphereBox(RigidBody& a, RigidBody& b, Contact& out) const;
    bool collideBoxBox(RigidBody& a, RigidBody& b, Contact& out) const;

    void resolve(const Contact& c) const;
    void positionCorrection(const Contact& c) const;
};