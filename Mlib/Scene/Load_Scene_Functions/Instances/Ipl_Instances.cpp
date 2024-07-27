#include "Ipl_Instances.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Dynamics.hpp>
#include <Mlib/Scene_Graph/Instantiation/Instance_Information.hpp>
#include <Mlib/Scene_Graph/Instantiation/Instantiate_Frames.hpp>
#include <Mlib/Scene_Graph/Instantiation/Read_Ipl.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(files);
DECLARE_ARGUMENT(except);
DECLARE_ARGUMENT(dynamics);
}

const std::string IplInstances::key = "ipl_instances";

LoadSceneJsonUserFunction IplInstances::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    IplInstances(args.renderable_scene()).execute(args);
};

IplInstances::IplInstances(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void IplInstances::execute(const LoadSceneJsonUserFunctionArgs &args) {
    auto empty_set = std::set<std::string>();
    auto exclude = args.arguments.at_non_null<std::set<std::string>>(KnownArgs::except, empty_set);
    auto dynamics = rendering_dynamics_from_string(args.arguments.at<std::string>(KnownArgs::dynamics));
    for (const auto& file : args.arguments.pathes_or_variables(KnownArgs::files)) {
        instantiate(
            scene,
            read_ipl(file.path, dynamics),
            scene_node_resources,
            RenderingContextStack::primary_rendering_resources(),
            exclude);
    }
}
