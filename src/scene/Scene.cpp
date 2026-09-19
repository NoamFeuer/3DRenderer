#include <algorithm>
#include <utility>

#include "Scene.hpp"

SceneNode& Scene::addNode(const std::string& name, SceneNode* parent) {
    auto node = std::make_unique<SceneNode>(name);
    SceneNode* raw = node.get();

    if (parent) {
        raw->parent = parent;
        parent->children.push_back(raw);
    }

    nodes.push_back(std::move(node));
    return *raw;
}

SceneNode* Scene::find(const std::string& name) {
    for (const auto& node : nodes)
        if (node->name == name)
            return node.get();
    return nullptr;
}

void Scene::removeNode(SceneNode& node) {
    if (node.parent) {
        std::vector<SceneNode*>& siblings = node.parent->children;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), &node), siblings.end());
    }

    // Children become roots (no automatic cascade delete).
    for (SceneNode* child : node.children)
        child->parent = nullptr;
    node.children.clear();
    node.parent = nullptr;

    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        if (it->get() == &node) {
            nodes.erase(it);
            break;
        }
    }
}

void Scene::clear() {
    nodes.clear();
}