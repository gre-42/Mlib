#include "Create_Yaw_Pitch_Lookat_Nodes.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
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
DECLARE_ARGUMENT(parent_follower_rigid_body_node);
DECLARE_ARGUMENT(followed);
DECLARE_ARGUMENT(head_node);
DECLARE_ARGUMENT(bullet_start_offset);
DECLARE_ARGUMENT(bullet_velocity);
DECLARE_ARGUMENT(bullet_feels_gravity);
DECLARE_ARGUMENT(gravity);
DECLARE_ARGUMENT(dyaw_max);
DECLARE_ARGUMENT(pitch_min);
DECLARE_ARGUMENT(pitch_max);
DECLARE_ARGUMENT(dpitch_max);
DECLARE_ARGUMENT(yaw_locked_on_max);
DECLARE_ARGUMENT(pitch_locked_on_max);
DECLARE_ARGUMENT(velocity_error_std);
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
    DanglingRef<SceneNode> yaw_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::yaw_node), DP_LOC);
    DanglingRef<SceneNode> pitch_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::pitch_node), DP_LOC);
    DanglingRef<SceneNode> follower_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::parent_follower_rigid_body_node), DP_LOC);
    auto follower_rb = dynamic_cast<RigidBodyVehicle*>(&follower_node->get_absolute_movable());
    if (follower_rb == nullptr) {
        THROW_OR_ABORT("Follower movable is not a rigid body");
    }
    DanglingPtr<SceneNode> followed_node = nullptr;
    RigidBodyVehicle* followed_rb = nullptr;
    if (args.arguments.contains(KnownArgs::followed)) {
        followed_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::followed), DP_LOC).ptr();
        followed_rb = dynamic_cast<RigidBodyVehicle*>(&followed_node->get_absolute_movable());
        if (followed_rb == nullptr) {
            THROW_OR_ABORT("Followed movable is not a rigid body");
        }
    }
    float velocity_error_std = args.arguments.at<float>(KnownArgs::velocity_error_std);
    float yaw_error_std = args.arguments.at<float>(KnownArgs::yaw_error_std);
    float pitch_velocity_error_std = args.arguments.at<float>(KnownArgs::pitch_error_std);
    float error_alpha = args.arguments.at<float>(KnownArgs::error_alpha);
    // octave> a=0.002; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9980
    // octave> a=0.004; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9960
    // => var = a / 2, std = sqrt(a / 2)
    auto velocity_estimation_error = RandomProcess<FastNormalRandomNumberGenerator<float>, ExponentialSmoother<float>>{
        FastNormalRandomNumberGenerator<float>{ 0, 0.f, velocity_error_std * std::sqrt(2.f / error_alpha) },
        ExponentialSmoother<float>{ error_alpha, velocity_error_std } };
    auto increment_yaw_error = RandomProcess<FastNormalRandomNumberGenerator<float>, ExponentialSmoother<float>>{
        FastNormalRandomNumberGenerator<float>{ 0, 0.f, yaw_error_std * std::sqrt(2.f / error_alpha) },
        ExponentialSmoother<float>{ error_alpha, yaw_error_std } };
    auto increment_pitch_error = RandomProcess<FastNormalRandomNumberGenerator<float>, ExponentialSmoother<float>>{
        FastNormalRandomNumberGenerator<float>{ 0, 0.f, pitch_velocity_error_std * std::sqrt(2.f / error_alpha) },
        ExponentialSmoother<float>{ error_alpha, pitch_velocity_error_std } };

    float bullet_start_offset  = args.arguments.at<float>(KnownArgs::bullet_start_offset) * meters;
    float bullet_velocity      = args.arguments.at<float>(KnownArgs::bullet_velocity) * meters / s;
    bool  bullet_feels_gravity = args.arguments.at<bool>(KnownArgs::bullet_feels_gravity);
    float gravity              = args.arguments.at<float>(KnownArgs::gravity) * meters / (s * s);

    auto follower_pitch = std::make_unique<PitchLookAtNode>(
        physics_engine.advance_times_,
        *follower_rb,
        bullet_start_offset,
        bullet_velocity,
        bullet_feels_gravity,
        gravity,
        args.arguments.at<float>(KnownArgs::pitch_min) * degrees,
        args.arguments.at<float>(KnownArgs::pitch_max) * degrees,
        args.arguments.at<float>(KnownArgs::dpitch_max) * degrees,
        args.arguments.at<float>(KnownArgs::pitch_locked_on_max) * degrees,
        velocity_estimation_error,
        increment_pitch_error);
    auto follower = std::make_unique<YawPitchLookAtNodes>(
        *follower_pitch,
        physics_engine.advance_times_,
        *follower_rb,
        bullet_start_offset,
        bullet_velocity,
        bullet_feels_gravity,
        gravity,
        args.arguments.at<float>(KnownArgs::dyaw_max) * degrees,
        args.arguments.at<float>(KnownArgs::yaw_locked_on_max) * degrees,
        velocity_estimation_error,
        increment_yaw_error,
        increment_pitch_error);
    follower->set_followed(followed_node, followed_rb);
    if (args.arguments.contains(KnownArgs::head_node)) {
        follower->pitch_look_at_node().set_head_node(scene.get_node(args.arguments.at<std::string>(KnownArgs::head_node), DP_LOC));
    }
    linker.link_relative_movable(yaw_node, std::move(follower));
    linker.link_relative_movable(pitch_node, std::move(follower_pitch));
}
