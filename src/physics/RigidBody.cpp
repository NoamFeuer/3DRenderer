#include <cmath>

#include "RigidBody.hpp"

namespace {
    Mat4 transpose3(const Mat4& m) {
        Mat4 t;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                t.m[i][j] = m.m[j][i];
        return t;
    }
}

RigidBody::RigidBody(SceneNode& node, Shape shape) : node(&node), shape(shape) {
    setMass(mass);
    updateInverseInertia();
}

void RigidBody::setMass(float m) {
    mass = m;
    invMass = (m > 0.0f) ? 1.0f / m : 0.0f;
    updateInverseInertia();
}

Vect3 RigidBody::getWorldCenter() const {
    return node->getWorldPosition();
}

float RigidBody::getWorldRadius() const {
    Vect3 center = getWorldCenter();
    Vect3 r = node->getWorldMatrix() * Vect3(radius, 0.0f, 0.0f);
    return (r - center).length();
}

void RigidBody::getWorldObb(Vect3& center, Vect3 axes[3], Vect3& halfExtentsWorld) const {
    center = getWorldCenter();

    const Vect3 localExtents[3] = {
        Vect3(halfExtents.x, 0.0f, 0.0f),
        Vect3(0.0f, halfExtents.y, 0.0f),
        Vect3(0.0f, 0.0f, halfExtents.z)
    };

    for (int i = 0; i < 3; i++) {
        Vect3 worldVec = node->getWorldMatrix() * localExtents[i] - center;
        float len = worldVec.length();
        axes[i] = len > 1e-8f ? worldVec * (1.0f / len) : Vect3(0.0f, 0.0f, 0.0f);
        if (i == 0) halfExtentsWorld.x = len;
        else if (i == 1) halfExtentsWorld.y = len;
        else halfExtentsWorld.z = len;
    }
}

void RigidBody::applyImpulse(const Vect3& j, const Vect3& contactPoint) {
    if (isStatic())
        return;

    velocity += j * invMass;

    Vect3 r = contactPoint - getWorldCenter();
    angularVelocity += invInertiaWorld * (r.cross(j));
}

void RigidBody::computeWorldInertia(float& invIx, float& invIy, float& invIz) {
    if (invMass == 0.0f) {
        invIx = invIy = invIz = 0.0f;
        return;
    }

    if (shape == Shape::Sphere) {
        float wr = getWorldRadius();
        float invI = 5.0f / (2.0f * mass * wr * wr);
        invIx = invIy = invIz = invI;
        return;
    }

    Vect3 h;
    {
        Vect3 center, axes[3];
        getWorldObb(center, axes, h);
    }

    // Solid box: I_axis = m * (b^2 + c^2) / 3 for the two sides not on that axis.
    float m = mass;
    invIx = 3.0f / (m * (h.y * h.y + h.z * h.z));
    invIy = 3.0f / (m * (h.x * h.x + h.z * h.z));
    invIz = 3.0f / (m * (h.x * h.x + h.y * h.y));
}

void RigidBody::updateInverseInertia() {
    float invIx, invIy, invIz;
    computeWorldInertia(invIx, invIy, invIz);

    if (invMass == 0.0f) {
        // Static bodies have a zero inertia tensor.
        invInertiaWorld = Mat4();
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                invInertiaWorld.m[i][j] = 0.0f;
        return;
    }

    Mat4 d; // diagonal world inverse inertia
    d.m[0][0] = invIx;
    d.m[1][1] = invIy;
    d.m[2][2] = invIz;

    Mat4 r = node->rotation.toMatrix();
    invInertiaWorld = r * d * transpose3(r);
}