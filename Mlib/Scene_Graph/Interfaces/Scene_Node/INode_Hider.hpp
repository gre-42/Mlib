#pragma once

namespace Mlib {

template <class T>
class DanglingRef;
class SceneNode;
struct ExternalRenderPass;

class INodeHider {
public:
    virtual bool node_shall_be_hidden(
        const DanglingRef<const SceneNode>& camera_node,
        const ExternalRenderPass& external_render_pass) const = 0;
};

}
