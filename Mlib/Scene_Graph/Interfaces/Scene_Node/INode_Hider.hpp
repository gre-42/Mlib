#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>

namespace Mlib {

template <class T>
class DanglingPtr;
class SceneNode;
struct ExternalRenderPass;

class INodeHider: public virtual DanglingBaseClass {
public:
    virtual bool node_shall_be_hidden(
        const DanglingBaseClassPtr<const SceneNode>& camera_node,
        const ExternalRenderPass& external_render_pass) const = 0;
};

}
