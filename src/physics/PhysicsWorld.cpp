#include <algorithm>
#include <cmath>
#include <limits>

#include "PhysicsWorld.hpp"

namespace {
    float clampf(float v, float lo, float hi) {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    struct Obb {
        Vect3 center;
        Vect3 axes[3];
        Vect3 halfExtents;
    };

    Obb makeObb(const RigidBody& body) {
        Obb box;
        body.getWorldObb(box.center, box.axes, box.halfExtents);
        return box;
    }

    // Half-width of a box projected onto a given axis.
    float comp(const Vect3& v, int i) {
        return i == 0 ? v.x : (i == 1 ? v.y : v.z);
    }

    float projectedRadius(const Obb& box, const Vect3& axis) {
        return box.halfExtents.x * std::abs(box.axes[0].dot(axis)) +
               box.halfExtents.y * std::abs(box.axes[1].dot(axis)) +
               box.halfExtents.z * std::abs(box.axes[2].dot(axis));
    }

    Vect3 extremeCorner(const Obb& box, const Vect3& dir, bool maximize) {
        Vect3 corner = box.center;
        for (int i = 0; i < 3; i++) {
            float proj = box.axes[i].dot(dir);
            bool positive = maximize ? (proj > 0.0f) : (proj < 0.0f);
            corner += box.axes[i] * (positive ? comp(box.halfExtents, i) : -comp(box.halfExtents, i));
        }
        return corner;
    }
}

void PhysicsWorld::add(RigidBody& body) {
    bodies.push_back(&body);
}

void PhysicsWorld::remove(RigidBody& body) {
    bodies.erase(std::remove(bodies.begin(), bodies.end(), &body), bodies.end());
}

void PhysicsWorld::clear() {
    bodies.clear();
    contacts.clear();
}

void PhysicsWorld::integrate(RigidBody& body, float dt) {
    if (body.isStatic())
        return;

    body.velocity += gravity * (body.gravityScale * dt);
    body.velocity *= std::max(0.0f, 1.0f - body.linearDamping * dt);
    body.angularVelocity *= std::max(0.0f, 1.0f - body.angularDamping * dt);

    body.node->position += body.velocity * dt;

    // Angular integration: dq/dt = 0.5 * (w as quat) * q.
    Quat q = body.node->rotation;
    Quat wq(body.angularVelocity.x, body.angularVelocity.y, body.angularVelocity.z, 0.0f);
    Quat dq = wq * q * (0.5f * dt);
    body.node->rotation = (q + dq).normalize();

    body.updateInverseInertia();
}

bool PhysicsWorld::collideSphereSphere(RigidBody& a, RigidBody& b, Contact& out) const {
    Vect3 centerA = a.getWorldCenter();
    Vect3 centerB = b.getWorldCenter();
    float radiusA = a.getWorldRadius();
    float radiusB = b.getWorldRadius();

    Vect3 delta = centerB - centerA;
    float distSq = delta.lengthSquared();
    float radiusSum = radiusA + radiusB;
    if (distSq >= radiusSum * radiusSum)
        return false;

    float dist = std::sqrt(distSq);
    out.a = &a;
    out.b = &b;
    out.normal = dist > 1e-8f ? delta * (1.0f / dist) : Vect3(0.0f, 1.0f, 0.0f);
    out.penetration = radiusSum - dist;
    out.point = centerA + out.normal * (radiusA - out.penetration * 0.5f);
    return true;
}

bool PhysicsWorld::collideSphereBox(RigidBody& sphere, RigidBody& box, Contact& out) const {
    Vect3 sphereCenter = sphere.getWorldCenter();
    float sphereRadius = sphere.getWorldRadius();
    Obb obb = makeObb(box);

    Vect3 delta = sphereCenter - obb.center;
    Vect3 closest = obb.center;
    bool inside = true;
    float dist[3];
    for (int i = 0; i < 3; i++) {
        dist[i] = delta.dot(obb.axes[i]);
        if (std::abs(dist[i]) > comp(obb.halfExtents, i)) {
            float clamped = clampf(dist[i], -comp(obb.halfExtents, i), comp(obb.halfExtents, i));
            closest += obb.axes[i] * clamped;
            inside = false;
        }
        else {
            closest += obb.axes[i] * dist[i];
        }
    }

    out.a = &sphere;
    out.b = &box;

    if (inside) {
        // Sphere center is inside the box: push out along the nearest face.
        int bestAxis = 0;
        float bestPen = std::numeric_limits<float>::infinity();
        for (int i = 0; i < 3; i++) {
            float pen = comp(obb.halfExtents, i) - std::abs(dist[i]);
            if (pen < bestPen) {
                bestPen = pen;
                bestAxis = i;
            }
        }
        // Normal points from the sphere (a) toward the box (b), so the positive
        // impulse pushes the sphere out of the box.
        Vect3 normal = obb.axes[bestAxis] * (dist[bestAxis] >= 0.0f ? -1.0f : 1.0f);
        out.normal = normal;
        out.penetration = sphereRadius + bestPen;
        out.point = sphereCenter - out.normal * sphereRadius;
        return true;
    }

    Vect3 centerDelta = sphereCenter - closest;
    float distToClosest = centerDelta.length();
    if (distToClosest >= sphereRadius)
        return false;

    // Normal points from the sphere (a) toward the box (b).
    out.normal = distToClosest > 1e-8f ? (closest - sphereCenter) * (1.0f / distToClosest)
                                       : Vect3(0.0f, -1.0f, 0.0f);
    out.penetration = sphereRadius - distToClosest;
    out.point = closest;
    return true;
}

bool PhysicsWorld::collideBoxBox(RigidBody& a, RigidBody& b, Contact& out) const {
    Obb boxA = makeObb(a);
    Obb boxB = makeObb(b);

    Vect3 bestAxis = Vect3(0.0f, 0.0f, 0.0f);
    float bestPen = std::numeric_limits<float>::infinity();
    int bestOwnership = -1; // 0 from A's face, 1 from B's face, 2 from an edge pair

    const auto testAxis = [&](const Vect3& axis, int ownership) {
        float len = axis.length();
        if (len < 1e-8f)
            return true;
        Vect3 n = axis * (1.0f / len);

        float pen = projectedRadius(boxA, n) + projectedRadius(boxB, n)
                  - std::abs((boxB.center - boxA.center).dot(n));
        if (pen <= 0.0f)
            return false; // separating axis found

        if (pen < bestPen) {
            bestPen = pen;
            bestAxis = n;
            bestOwnership = ownership;
        }
        return true;
    };

    // Face normals of both boxes.
    for (int i = 0; i < 3; i++)
        if (!testAxis(boxA.axes[i], 0)) return false;
    for (int i = 0; i < 3; i++)
        if (!testAxis(boxB.axes[i], 1)) return false;

    // All pairwise edge cross products.
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (!testAxis(boxA.axes[i].cross(boxB.axes[j]), 2))
                return false;

    // Ensure the normal points from a toward b.
    if ((boxB.center - boxA.center).dot(bestAxis) < 0.0f)
        bestAxis = bestAxis * -1.0f;

    out.a = &a;
    out.b = &b;
    out.normal = bestAxis;
    out.penetration = bestPen;

    if (bestOwnership == 0) {
        // Deepest corner of B inside A along the normal.
        out.point = extremeCorner(boxB, bestAxis, false);
    }
    else if (bestOwnership == 1) {
        // Deepest corner of A inside B (normal points from A to B).
        out.point = extremeCorner(boxA, bestAxis, true);
    }
    else {
        out.point = (boxA.center + boxB.center) * 0.5f;
    }
    return true;
}

bool PhysicsWorld::collide(RigidBody& a, RigidBody& b, Contact& out) const {
    if (a.shape == RigidBody::Shape::Sphere && b.shape == RigidBody::Shape::Sphere)
        return collideSphereSphere(a, b, out);
    if (a.shape == RigidBody::Shape::Sphere && b.shape == RigidBody::Shape::Box)
        return collideSphereBox(a, b, out);
    if (a.shape == RigidBody::Shape::Box && b.shape == RigidBody::Shape::Sphere)
        return collideSphereBox(b, a, out);
    return collideBoxBox(a, b, out);
}

void PhysicsWorld::resolve(const Contact& c) const {
    RigidBody* a = c.a;
    RigidBody* b = c.b;
    if (a->isStatic() && b->isStatic())
        return;

    Vect3 centerA = a->getWorldCenter();
    Vect3 centerB = b->getWorldCenter();
    Vect3 ra = c.point - centerA;
    Vect3 rb = c.point - centerB;

    Vect3 va = a->velocity + a->angularVelocity.cross(ra);
    Vect3 vb = b->velocity + b->angularVelocity.cross(rb);
    Vect3 rel = vb - va;
    float velN = rel.dot(c.normal);
    if (velN > 0.0f)
        return; // already separating

    Vect3 raxn = ra.cross(c.normal);
    Vect3 rbxn = rb.cross(c.normal);
    float kNormal = a->invMass + b->invMass
                  + raxn.dot(a->invInertiaWorld * raxn)
                  + rbxn.dot(b->invInertiaWorld * rbxn);
    if (kNormal < 1e-8f)
        return;

    float e = std::min(a->restitution, b->restitution);
    float jn = -(1.0f + e) * velN / kNormal;
    Vect3 normalImpulse = c.normal * jn;

    a->applyImpulse(normalImpulse * -1.0f, c.point);
    b->applyImpulse(normalImpulse, c.point);

    // Coulomb friction against the tangential relative velocity.
    Vect3 tangent = rel - c.normal * velN;
    float tangentLen = tangent.length();
    if (tangentLen > 1e-6f) {
        Vect3 tangentDir = tangent * (1.0f / tangentLen);
        float mu = std::sqrt(a->friction * b->friction);
        float jtRaw = -tangentLen / kNormal;
        float jt = clampf(jtRaw, -mu * jn, mu * jn);
        Vect3 frictionImpulse = tangentDir * jt;
        a->applyImpulse(frictionImpulse * -1.0f, c.point);
        b->applyImpulse(frictionImpulse, c.point);
    }
}

void PhysicsWorld::positionCorrection(const Contact& c) const {
    RigidBody* a = c.a;
    RigidBody* b = c.b;

    const float percent = 0.6f;
    const float slop = 0.005f;
    if (c.penetration <= slop)
        return;

    float totalInvMass = a->invMass + b->invMass;
    if (totalInvMass < 1e-8f)
        return;

    Vect3 correction = c.normal * (percent * (c.penetration - slop) / totalInvMass);
    if (!a->isStatic())
        a->node->position -= correction * a->invMass;
    if (!b->isStatic())
        b->node->position += correction * b->invMass;
}

void PhysicsWorld::detectCollisions() {
    contacts.clear();
    for (size_t i = 0; i < bodies.size(); i++) {
        for (size_t j = i + 1; j < bodies.size(); j++) {
            RigidBody* a = bodies[i];
            RigidBody* b = bodies[j];
            if (a->isStatic() && b->isStatic())
                continue;

            Contact c{ a, b, Vect3(0.0f, 1.0f, 0.0f), 0.0f, Vect3(0.0f, 0.0f, 0.0f) };
            if (collide(*a, *b, c))
                contacts.push_back(c);
        }
    }
}

void PhysicsWorld::step(float dt) {
    float subDt = dt / static_cast<float>(substeps);
    for (int s = 0; s < substeps; s++) {
        for (RigidBody* body : bodies)
            integrate(*body, subDt);

        detectCollisions();

        for (const Contact& c : contacts) {
            resolve(c);
            positionCorrection(c);
        }
    }
}