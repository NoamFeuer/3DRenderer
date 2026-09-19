#pragma once

#include <memory>
#include <string>
#include <vector>

#include "SceneNode.hpp"

// Owns every SceneNode in the world. Nodes reference each other through
// SceneNode's parent/children links, which never own memory, so all nodes are
// destroyed together when the Scene goes away.
class Scene {
public:
    Scene() = default;
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // Creates a node named `name`; if parent != nullptr it is linked under
    // parent, otherwise it becomes a root.
    SceneNode& addNode(const std::string& name, SceneNode* parent = nullptr);

    SceneNode* find(const std::string& name);
    void removeNode(SceneNode& node);

    size_t size() const { return nodes.size(); }
    void clear();
    bool empty() const { return nodes.empty(); }

private:
    std::vector<std::unique_ptr<SceneNode>> nodes;
};