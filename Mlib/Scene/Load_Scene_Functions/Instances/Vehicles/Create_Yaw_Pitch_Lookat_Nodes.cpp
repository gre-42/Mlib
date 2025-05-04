#include "Create_Yaw_Pitch_Lookat_Nodes.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Aim_At.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Stats/Fast_Random_Number_Generators.hpp>
#include <Mlib/Stats/Random_Process.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(yaw_node);
DECLARE_ARGUMENT(pitch_node);
DECLARE_ARGUMENT(gun_node);
DECLARE_ARGUMENT(head_node);
DECLARE_ARGUMENT(dyaw_max);
DECLARE_ARGUMENT(pitch_min);
DECLARE_ARGUMENT(pitch_max);
DECLARE_ARGUMENT(dpitch_max);
DECLARE_ARGUMENT(yaw_error_std);
DECLARE_ARGUMENT(pitch_error_std);
DECLARE_ARGUMENT(error_alpha);
DECLARE_ARGUMENT(dpitch_head);
}

const std::string CreateYawPitchLookatNodes::key = "yaw_pitch_look_at_nodes";

LoadSceneJsonUserFunction CreateYawPitchLookatNodes::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateYawPitchLookatNodes(args.renderable_scene()).execute(args);
};

CreateYawPitchLookatNodes::CreateYawPitchLookatNodes(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateYawPitchLookatNodes::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    DanglingRef<SceneNode> yaw_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::yaw_node), DP_LOC);
    DanglingRef<SceneNode> pitch_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::pitch_node), DP_LOC);
    DanglingRef<SceneNode> gun_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::gun_node), DP_LOC);
    float yaw_error_std = args.arguments.at<float>(KnownArgs::yaw_error_std);
    float pitch_velocity_error_std = args.arguments.at<float>(KnownArgs::pitch_error_std);
    float error_alpha = (pitch_velocity_error_std != 0.f)
        ? args.arguments.at<float>(KnownArgs::error_alpha)
        : 1.f;
    // octave> a=0.002; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9980
    // octave> a=0.004; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9960
    // => var = a / 2, std = sqrt(a / 2)
    auto increment_yaw_error = RandomProcess<FastNormalRandomNumberGenerator<float>, ExponentialSmoother<float>>{
        FastNormalRandomNumberGenerator<float>{ 0, 0.f, yaw_error_std * std::sqrt(2.f / error_alpha) },
        ExponentialSmoother<float>{ error_alpha, yaw_error_std } };
    auto increment_pitch_error = RandomProcess<FastNormalRandomNumberGenerator<float>, ExponentialSmoother<float>>{
        FastNormalRandomNumberGenerator<float>{ 0, 0.f, pitch_velocity_error_std * std::sqrt(2.f / error_alpha) },
        ExponentialSmoother<float>{ error_alpha, pitch_velocity_error_std } };

    auto& aim_at = get_aim_at(gun_node);
    auto follower_pitch = std::make_unique<PitchLookAtNode>(
        aim_at,
        args.arguments.at<float>(KnownArgs::pitch_min) * degrees,
        args.arguments.at<float>(KnownArgs::pitch_max) * degrees,
        args.arguments.at<float>(KnownArgs::dpitch_max) * degrees / integral_to_float<float>(scene_config.physics_engine_config.nsubsteps),
        increment_pitch_error);
    auto follower = std::make_unique<YawPitchLookAtNodes>(
        aim_at,
        *follower_pitch,
        args.arguments.at<float>(KnownArgs::dyaw_max) * degrees / integral_to_float<float>(scene_config.physics_engine_config.nsubsteps),
        increment_yaw_error);
    if (args.arguments.contains(KnownArgs::head_node)) {
        follower->pitch_look_at_node().set_head_node(scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::head_node), DP_LOC));
    }
    linker.link_relative_movable(
        yaw_node,
        DanglingBaseClassRef<YawPitchLookAtNodes>{ *follower, CURRENT_SOURCE_LOCATION },
        CURRENT_SOURCE_LOCATION);
    linker.link_relative_movable(
        pitch_node,
        DanglingBaseClassRef<PitchLookAtNode>{ *follower_pitch, CURRENT_SOURCE_LOCATION },
        CURRENT_SOURCE_LOCATION);
    global_object_pool.add(std::move(follower), CURRENT_SOURCE_LOCATION);
    global_object_pool.add(std::move(follower_pitch), CURRENT_SOURCE_LOCATION);
}
