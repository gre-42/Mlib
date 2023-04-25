#include "Renderable_Instance.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Game_Logic/Supply_Depots.hpp>
#include <Mlib/Render/Imposters.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resource.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(include);
DECLARE_ARGUMENT(exclude);
}

const std::string RenderableInstance::key = "renderable_instance";

LoadSceneJsonUserFunction RenderableInstance::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    RenderableInstance(args.renderable_scene()).execute(args);
};

RenderableInstance::RenderableInstance(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RenderableInstance::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    scene_node_resources.instantiate_renderable(
        args.arguments.at<std::string>(KnownArgs::resource),
        InstantiationOptions{
            .imposters = &imposters,
            .supply_depots = &supply_depots,
            .instance_name = args.arguments.at<std::string>(KnownArgs::name),
            .scene_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node)),
            .renderable_resource_filter = RenderableResourceFilter {
                .cva_filter = {
                    .included_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::include, "")),
                    .excluded_names = Mlib::compile_regex(args.arguments.at<std::string>(KnownArgs::exclude, "$ ^"))}}});
}
