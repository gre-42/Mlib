#include "Child_Renderable_Instance.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Interfaces/IScene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
}

ChildRenderableInstance::ChildRenderableInstance(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void ChildRenderableInstance::execute(const LoadSceneJsonUserFunctionArgs& args) const
{
    args.arguments.validate(KnownArgs::options);
    (*this)(
        args.arguments.at<std::string>(KnownArgs::name),
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node),
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::resource),
        RenderableResourceFilter{
            .cva_filter = {
                .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
                .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))} });
}

void ChildRenderableInstance::operator () (
    const std::string& instance_name,
    const VariableAndHash<std::string>& node,
    const VariableAndHash<std::string>& resource,
    const RenderableResourceFilter& resource_filter) const
{
    scene_node_resources.instantiate_child_renderable(
        resource,
        ChildInstantiationOptions{
            .rendering_resources = &rendering_resources,
            .instance_name = VariableAndHash{ instance_name },
            .scene_node = scene.get_node(node, DP_LOC),
            .renderable_resource_filter = resource_filter });
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "child_renderable_instance",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ChildRenderableInstance(args.physics_scene()).execute(args);
            });
    }
} obj;

}
