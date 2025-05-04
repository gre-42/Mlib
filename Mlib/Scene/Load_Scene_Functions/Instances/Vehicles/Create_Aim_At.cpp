#include "Create_Aim_At.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
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
DECLARE_ARGUMENT(gun_node);
DECLARE_ARGUMENT(parent_follower_rigid_body_node);
DECLARE_ARGUMENT(followed);
DECLARE_ARGUMENT(bullet_start_offset);
DECLARE_ARGUMENT(bullet_velocity);
DECLARE_ARGUMENT(bullet_feels_gravity);
DECLARE_ARGUMENT(gravity);
DECLARE_ARGUMENT(locked_on_angle);
DECLARE_ARGUMENT(velocity_error_std);
DECLARE_ARGUMENT(error_alpha);
}

const std::string CreateAimAt::key = "aim_at";

LoadSceneJsonUserFunction CreateAimAt::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateAimAt(args.renderable_scene()).execute(args);
};

CreateAimAt::CreateAimAt(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAimAt::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    DanglingRef<SceneNode> gun_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::gun_node), DP_LOC);
    DanglingRef<SceneNode> follower_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::parent_follower_rigid_body_node), DP_LOC);
    DanglingPtr<SceneNode> followed_node = nullptr;
    if (args.arguments.contains(KnownArgs::followed)) {
        followed_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::followed), DP_LOC).ptr();
    }
    float velocity_error_std = args.arguments.at<float>(KnownArgs::velocity_error_std);
    float error_alpha = (velocity_error_std != 0.f)
        ? args.arguments.at<float>(KnownArgs::error_alpha)
        : 1.f;
    // octave> a=0.002; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9980
    // octave> a=0.004; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9960
    // => var = a / 2, std = sqrt(a / 2)
    auto velocity_estimation_error = RandomProcess<FastNormalRandomNumberGenerator<float>, ExponentialSmoother<float>>{
        FastNormalRandomNumberGenerator<float>{ 0, 0.f, velocity_error_std * std::sqrt(2.f / error_alpha) },
        ExponentialSmoother<float>{ error_alpha, velocity_error_std } };

    float bullet_start_offset  = args.arguments.at<float>(KnownArgs::bullet_start_offset) * meters;
    float bullet_velocity      = args.arguments.at<float>(KnownArgs::bullet_velocity) * meters / seconds;
    bool  bullet_feels_gravity = args.arguments.at<bool>(KnownArgs::bullet_feels_gravity);
    float gravity              = args.arguments.at<float>(KnownArgs::gravity) * meters / (seconds * seconds);

    auto& aim_at = global_object_pool.create<AimAt>(
        CURRENT_SOURCE_LOCATION,
        physics_engine.advance_times_,
        follower_node,
        gun_node,
        bullet_start_offset,
        bullet_velocity,
        bullet_feels_gravity,
        gravity,
        std::cos(args.arguments.at<float>(KnownArgs::locked_on_angle) * degrees),
        velocity_estimation_error);
    aim_at.set_followed(followed_node);
}
