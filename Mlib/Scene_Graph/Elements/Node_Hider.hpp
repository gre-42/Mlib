#pragma once

namespace Mlib {

class SceneNode;

class NodeHider {
public:
    virtual bool node_shall_be_hidden(const SceneNode& camera_node) const = 0;
};

}
