#pragma once
#include <Mlib/Scene/Json_User_Function.hpp>
#include <Mlib/Scene/Load_Scene_Instance_Function.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>

namespace Mlib {

struct RenderableResourceFilter;

class ChildRenderableInstance: public LoadSceneInstanceFunction {
public:
    static LoadSceneJsonUserFunction json_user_function;
    static const std::string key;
    explicit ChildRenderableInstance(RenderableScene& renderable_scene);
    void operator () (
        const std::string& instance_name,
        const std::string& node,
        const std::string& resource,
        const RenderableResourceFilter& resource_filter = RenderableResourceFilter{}) const;
private:
    void execute(const LoadSceneJsonUserFunctionArgs& args) const;
};

}
