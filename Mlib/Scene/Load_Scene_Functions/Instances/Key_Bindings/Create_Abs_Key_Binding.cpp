#include "Create_Abs_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Key_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(role);

DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);

DECLARE_ARGUMENT(force);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(rotate);
DECLARE_ARGUMENT(car_surface_power);
DECLARE_ARGUMENT(max_velocity);
DECLARE_ARGUMENT(tire_id);
DECLARE_ARGUMENT(tire_angle_velocities);
DECLARE_ARGUMENT(tire_angles);
DECLARE_ARGUMENT(tires_z);
DECLARE_ARGUMENT(wants_to_jump);
DECLARE_ARGUMENT(wants_to_grind);
DECLARE_ARGUMENT(fly_forward_factor);
}

const std::string CreateAbsKeyBinding::key = "abs_key_binding";

static float from_kph(float v) {
    return v * kph;
}

static float from_degrees(float v) {
    return v * degrees;
}

LoadSceneJsonUserFunction CreateAbsKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateAbsKeyBinding(args.renderable_scene()).execute(args);
};

CreateAbsKeyBinding::CreateAbsKeyBinding(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateAbsKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& rb = get_rigid_body_vehicle(node);
    auto& kb = key_bindings.add_absolute_movable_key_binding(std::unique_ptr<AbsoluteMovableKeyBinding>(new AbsoluteMovableKeyBinding{
        .node = node.ptr(),
        .force = {
            .vector = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::force, fixed_zeros<float, 3>()) * N,
            .position = args.arguments.at<UFixedArray<ScenePos, 3>>(KnownArgs::position, rb.rbp_.com_.casted<ScenePos>()) * (ScenePos)meters},
        .rotate = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::rotate, fixed_zeros<float, 3>()),
        .car_surface_power = args.arguments.contains(KnownArgs::car_surface_power)
            ? args.arguments.at<float>(KnownArgs::car_surface_power) * W
            : std::optional<float>(),
        .max_velocity = args.arguments.at<float>(KnownArgs::max_velocity, INFINITY) * meters / seconds,
        .tire_id = args.arguments.at<size_t>(KnownArgs::tire_id, SIZE_MAX),
        .tire_angle_interp = Interp<float>{
            args.arguments.at_vector_non_null_optional<float>(KnownArgs::tire_angle_velocities, from_kph),
            args.arguments.at_vector_non_null_optional<float>(KnownArgs::tire_angles, from_degrees),
            OutOfRangeBehavior::CLAMP},
        .tires_z = args.arguments.at<UFixedArray<float, 3>>(KnownArgs::tires_z, fixed_zeros<float, 3>()),
        .wants_to_jump = args.arguments.contains(KnownArgs::wants_to_jump)
            ? args.arguments.at<bool>(KnownArgs::wants_to_jump)
            : std::optional<bool>(),
        .wants_to_grind = args.arguments.contains(KnownArgs::wants_to_grind)
            ? args.arguments.at<bool>(KnownArgs::wants_to_grind)
            : std::optional<bool>(),
        .fly_forward_factor = args.arguments.contains(KnownArgs::fly_forward_factor)
            ? args.arguments.at<float>(KnownArgs::fly_forward_factor) * N
            : std::optional<float>(),
        .button_press{
            args.button_states,
            args.key_configurations,
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::role)},
        .on_node_clear{ DestructionFunctionsRemovalTokens{ node->on_clear, CURRENT_SOURCE_LOCATION } },
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } }}));
    kb.on_node_clear.add([&kbs=key_bindings, &kb](){ kbs.delete_absolute_movable_key_binding(kb); }, CURRENT_SOURCE_LOCATION);
    kb.on_player_delete_vehicle_internals.add([&kbs=key_bindings, &kb](){ kbs.delete_absolute_movable_key_binding(kb); }, CURRENT_SOURCE_LOCATION);
}
