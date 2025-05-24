#include "Create_Car_Controller_Idle_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Car_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(surface_power);
DECLARE_ARGUMENT(steer_angle);
DECLARE_ARGUMENT(drive_relaxation);
DECLARE_ARGUMENT(steer_relaxation);
}

const std::string CreateCarControllerIdleBinding::key = "car_controller_idle_binding";

LoadSceneJsonUserFunction CreateCarControllerIdleBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateCarControllerIdleBinding(args.renderable_scene()).execute(args);
};

CreateCarControllerIdleBinding::CreateCarControllerIdleBinding(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateCarControllerIdleBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& kb = key_bindings.add_car_controller_idle_binding(std::unique_ptr<CarControllerIdleBinding>(new CarControllerIdleBinding{
        .node = node.ptr(),
        .surface_power = args.arguments.at<float>(KnownArgs::surface_power, 0.f) * W,
        .steer_angle = args.arguments.at<float>(KnownArgs::steer_angle, 0.f) * degrees,
        .drive_relaxation = args.arguments.at<float>(KnownArgs::drive_relaxation, 0.f),
        .steer_relaxation = args.arguments.at<float>(KnownArgs::steer_relaxation, 0.f),
        .on_node_clear{ DestructionFunctionsRemovalTokens{ node->on_clear, CURRENT_SOURCE_LOCATION } },
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } }}));
    kb.on_node_clear.add([&kbs=key_bindings, &kb](){ kbs.delete_car_controller_idle_binding(kb); }, CURRENT_SOURCE_LOCATION);
    kb.on_player_delete_vehicle_internals.add([&kbs=key_bindings, &kb](){ kbs.delete_car_controller_idle_binding(kb); }, CURRENT_SOURCE_LOCATION);
}
