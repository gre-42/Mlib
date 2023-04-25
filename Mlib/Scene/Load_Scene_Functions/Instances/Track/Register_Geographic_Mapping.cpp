#include "Register_Geographic_Mapping.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(resource);
}

const std::string RegisterGeographicMapping::key = "register_geographic_mapping";

LoadSceneJsonUserFunction RegisterGeographicMapping::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    RegisterGeographicMapping(args.renderable_scene()).execute(args);
};

RegisterGeographicMapping::RegisterGeographicMapping(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RegisterGeographicMapping::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    
    auto& node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    args.scene_node_resources.register_geographic_mapping(
        args.arguments.at<std::string>(KnownArgs::resource),
        args.arguments.at<std::string>(KnownArgs::name),
        node.absolute_model_matrix().casted<double, double>());
}
