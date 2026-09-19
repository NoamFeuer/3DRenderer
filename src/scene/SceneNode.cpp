#include <algorithm>
#include <utility>

#include "SceneNode.hpp"

SceneNode::SceneNode(std::string name) : name(std::move(name)) {}

void SceneNode::setParent(SceneNode* newParent) {
    if (parent == newParent)
        return;

    if (parent) {
        std::vector<SceneNode*>& siblings = parent->children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
    }

    if (newParent)
        newParent->children.push_back(this);

    parent = newParent;
}

Mat4 SceneNode::getLocalMatrix() const {
    return Mat4::translation(position.x, position.y, position.z) *
           rotation.toMatrix() *
           Mat4::scale(scale.x, scale.y, scale.z);
}

Mat4 SceneNode::getWorldMatrix() const {
    if (parent)
        return parent->getWorldMatrix() * getLocalMatrix();
    return getLocalMatrix();
}

Vect3 SceneNode::getWorldPosition() const {
    return getWorldMatrix() * Vect3(0.0f, 0.0f, 0.0f);
}

Quat SceneNode::getWorldRotation() const {
    if (parent)
        return parent->getWorldRotation() * rotation;
    return rotation;
}

float SceneNode::getWorldScale() const {
    float local = (scale.x + scale.y + scale.z) / 3.0f;
    if (parent)
        return parent->getWorldScale() * local;
    return local;
}

SceneNode* SceneNode::find(const std::string& nodeName) {
    if (name == nodeName)
        return this;
    for (SceneNode* child : children) {
        if (SceneNode* result = child->find(nodeName))
            return result;
    }
    return nullptr;
}

const SceneNode* SceneNode::find(const std::string& nodeName) const {
    if (name == nodeName)
        return this;
    for (const SceneNode* child : children) {
        if (const SceneNode* result = child->find(nodeName))
            return result;
    }
    return nullptr;
}