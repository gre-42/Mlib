#pragma once
#include <Mlib/Source_Location.hpp>
#include <string>

namespace Mlib {

class Scene;
template <class T>
class VariableAndHash;
class SceneNode;
template <class T>
class DanglingRef;

class INodeSetter {
public:
    virtual void set_scene_node(
        Scene& scene,
        const DanglingBaseClassRef<SceneNode>& node,
        VariableAndHash<std::string> node_name,
        SourceLocation loc) = 0;
};

}
