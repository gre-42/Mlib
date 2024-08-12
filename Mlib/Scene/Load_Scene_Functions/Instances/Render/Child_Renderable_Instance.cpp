#include "Child_Renderable_Instance.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Render/Imposters.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
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

const std::string ChildRenderableInstance::key = "child_renderable_instance";

LoadSceneJsonUserFunction ChildRenderableInstance::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    ChildRenderableInstance(args.renderable_scene()).execute(args);
};

ChildRenderableInstance::ChildRenderableInstance(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void ChildRenderableInstance::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene_node_resources.instantiate_child_renderable(
        args.arguments.at<std::string>(KnownArgs::resource),
        ChildInstantiationOptions{
            .rendering_resources = &rendering_resources,
            .instance_name = args.arguments.at<std::string>(KnownArgs::name),
            .scene_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC),
            .renderable_resource_filter = RenderableResourceFilter {
                .cva_filter = {
                    .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::included_names, "")),
                    .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::excluded_names, "$ ^"))}}});
}
