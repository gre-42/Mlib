#pragma once
#include <Mlib/Scene/Load_Physics_Scene_Instance_Function.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>

namespace Mlib {

template <class T>
class VariableAndHash;
struct LoadSceneJsonUserFunctionArgs;
struct RenderableResourceFilter;

class ChildRenderableInstance: public LoadPhysicsSceneInstanceFunction {
public:
    explicit ChildRenderableInstance(PhysicsScene& physics_scene);
    void operator () (
        const std::string& instance_name,
        const VariableAndHash<std::string>& node,
        const VariableAndHash<std::string>& resource,
        const RenderableResourceFilter& resource_filter = RenderableResourceFilter{}) const;
    void execute(const LoadSceneJsonUserFunctionArgs& args) const;
};

}
