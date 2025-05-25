#include "Root_Node_Instance.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Parse_Position.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(dynamics);
DECLARE_ARGUMENT(strategy);
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(scale);
DECLARE_ARGUMENT(interpolation);
DECLARE_ARGUMENT(user_id);
}

RootNodeInstance::RootNodeInstance(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void RootNodeInstance::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    // root nodes do not have a default pose
    FixedArray<CompressedScenePos, 3> pos = parse_position(
        args.arguments.at(KnownArgs::position),
        scene_node_resources);
    auto node = make_unique_scene_node(
        pos.casted<ScenePos>() * (ScenePos)meters,
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::rotation) * degrees,
        args.arguments.at<float>(KnownArgs::scale, 1.f),
        pose_interpolation_mode_from_string(args.arguments.at<std::string>(KnownArgs::interpolation, "enabled")),
        SceneNodeDomain::RENDER | SceneNodeDomain::PHYSICS,
        args.arguments.at<uint32_t>(KnownArgs::user_id, UINT32_MAX));
    auto rendering_dynamics = rendering_dynamics_from_string(args.arguments.at<std::string>(KnownArgs::dynamics, "moving"));
    auto rendering_strategy = rendering_strategy_from_string(args.arguments.at<std::string>(KnownArgs::strategy, "object"));
    scene.add_root_node(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
        std::move(node),
        rendering_dynamics,
        rendering_strategy);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "root_node_instance",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                RootNodeInstance(args.physics_scene()).execute(args);
            });
    }
} obj;

}
