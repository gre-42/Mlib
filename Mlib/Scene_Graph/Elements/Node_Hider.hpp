#pragma once

namespace Mlib {

class SceneNode;
struct ExternalRenderPass;

class NodeHider {
public:
    virtual bool node_shall_be_hidden(
        const SceneNode& camera_node,
        const ExternalRenderPass& external_render_pass) const = 0;
};

}
