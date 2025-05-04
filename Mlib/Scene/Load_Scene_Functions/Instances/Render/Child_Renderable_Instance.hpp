#pragma once
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>

namespace Mlib {

template <class T>
class VariableAndHash;
struct LoadSceneJsonUserFunctionArgs;
struct RenderableResourceFilter;

class ChildRenderableInstance: public LoadSceneInstanceFunction {
public:
    explicit ChildRenderableInstance(RenderableScene& renderable_scene);
    void operator () (
        const std::string& instance_name,
        const VariableAndHash<std::string>& node,
        const VariableAndHash<std::string>& resource,
        const RenderableResourceFilter& resource_filter = RenderableResourceFilter{}) const;
    void execute(const LoadSceneJsonUserFunctionArgs& args) const;
};

}
