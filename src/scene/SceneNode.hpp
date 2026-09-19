#pragma once

#include <string>
#include <vector>

#include "../math/Mat4.hpp"
#include "../math/Vect3.hpp"
#include "../math/Quat.hpp"

// A node in the scene graph. Holds a local transform (position + rotation +
// scale) plus a non-owning pointer to a parent and to children. Scene owns
// all node memory; these links are for traversal only.
class SceneNode {
public:
    explicit SceneNode(std::string name = "node");

    std::string name;

    Vect3 position{ 0.0f, 0.0f, 0.0f };
    Quat rotation;
    Vect3 scale{ 1.0f, 1.0f, 1.0f };

    SceneNode* parent = nullptr;
    std::vector<SceneNode*> children;

    // Reparents this node (updates the old and new parent's child lists).
    void setParent(SceneNode* newParent);

    Mat4 getLocalMatrix() const;
    Mat4 getWorldMatrix() const;

    Vect3 getWorldPosition() const;
    Quat getWorldRotation() const;
    // Approximate world-space scale magnitude, ignoring shear.
    float getWorldScale() const;

    Vect3 getWorldForward() const { return getWorldRotation().rotate(Vect3(0.0f, 0.0f, -1.0f)); }
    Vect3 getWorldRight() const { return getWorldRotation().rotate(Vect3(1.0f, 0.0f, 0.0f)); }
    Vect3 getWorldUp() const { return getWorldRotation().rotate(Vect3(0.0f, 1.0f, 0.0f)); }

    SceneNode* find(const std::string& nodeName);
    const SceneNode* find(const std::string& nodeName) const;
};