#pragma once

namespace Mlib {

template <class T>
class DanglingPtr;
class SceneNode;
struct ExternalRenderPass;

class INodeHider {
public:
    virtual bool node_shall_be_hidden(
        const DanglingPtr<const SceneNode>& camera_node,
        const ExternalRenderPass& external_render_pass) const = 0;
};

}
