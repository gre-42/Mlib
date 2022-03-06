#include "Create_Yaw_Pitch_Lookat_Nodes.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <Mlib/Stats/Random_Process.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(YAW_NODE);
DECLARE_OPTION(PITCH_NODE);
DECLARE_OPTION(PARENT_FOLLOWER_RIGID_BODY_NODE);
DECLARE_OPTION(FOLLOWED);
DECLARE_OPTION(BULLET_START_OFFSET);
DECLARE_OPTION(BULLET_VELOCITY);
DECLARE_OPTION(GRAVITY);
DECLARE_OPTION(DYAW_MAX);
DECLARE_OPTION(PITCH_MIN);
DECLARE_OPTION(PITCH_MAX);
DECLARE_OPTION(DPITCH_MAX);
DECLARE_OPTION(YAW_LOCKED_ON_MAX);
DECLARE_OPTION(PITCH_LOCKED_ON_MAX);
DECLARE_OPTION(VERROR_STD);
DECLARE_OPTION(VERROR_ALPHA);

LoadSceneUserFunction CreateYawPitchLookatNodes::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*yaw_pitch_look_at_nodes"
        "\\s+yaw_node=([\\w+-.]+)"
        "\\s+pitch_node=([\\w+-.]+)"
        "\\s+parent_follower_rigid_body_node=([\\w+-.]+)"
        "\\s+followed=([\\w+-.]*)"
        "\\s+bullet_start_offset=([\\w+-.]+)"
        "\\s+bullet_velocity=([\\w+-.]+)"
        "\\s+gravity=([\\w+-.]+)"
        "\\s+dyaw_max=([\\w+-.]+)"
        "\\s+pitch_min=([\\w+-.]+)"
        "\\s+pitch_max=([\\w+-.]+)"
        "\\s+dpitch_max=([\\w+-.]+)"
        "\\s+yaw_locked_on_max=([\\w+-.]+)"
        "\\s+pitch_locked_on_max=([\\w+-.]+)"
        "\\s+verror_std=([\\w+-.]+)"
        "\\s+verror_alpha=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateYawPitchLookatNodes(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateYawPitchLookatNodes::CreateYawPitchLookatNodes(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateYawPitchLookatNodes::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& yaw_node = scene.get_node(match[YAW_NODE].str());
    auto& pitch_node = scene.get_node(match[PITCH_NODE].str());
    auto& follower_node = scene.get_node(match[PARENT_FOLLOWER_RIGID_BODY_NODE].str());
    auto follower_rb = dynamic_cast<RigidBodyVehicle*>(follower_node.get_absolute_movable());
    if (follower_rb == nullptr) {
        throw std::runtime_error("Follower movable is not a rigid body");
    }
    SceneNode* followed_node = nullptr;
    RigidBodyVehicle* followed_rb = nullptr;
    if (!match[FOLLOWED].str().empty()) {
        followed_node = &scene.get_node(match[FOLLOWED].str());
        followed_rb = dynamic_cast<RigidBodyVehicle*>(followed_node->get_absolute_movable());
        if (followed_rb == nullptr) {
            throw std::runtime_error("Followed movable is not a rigid body");
        }
    }
    float verror_std = safe_stof(match[VERROR_STD].str());
    float verror_alpha = safe_stof(match[VERROR_ALPHA].str());
    // octave> a=0.002; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9980
    // octave> a=0.004; a/sum((a * (1 - a).^(0 : 100000)).^2)
    // ans = 1.9960
    // => var = a / 2, std = sqrt(a / 2)
    auto rp = RandomProcess<NormalRandomNumberGenerator<float>, ExponentialSmoother<float>>{
        NormalRandomNumberGenerator<float>{ 0, 0.f, verror_std * std::sqrt(2.f / verror_alpha) },
        ExponentialSmoother<float>{ verror_alpha, verror_std } };
    auto follower = std::make_shared<YawPitchLookAtNodes>(
        physics_engine.advance_times_,
        *follower_rb,
        safe_stof(match[BULLET_START_OFFSET].str()),
        safe_stof(match[BULLET_VELOCITY].str()),
        safe_stof(match[GRAVITY].str()),
        safe_stof(match[DYAW_MAX].str()) / 180.f * float(M_PI),
        safe_stof(match[PITCH_MIN].str()) / 180.f * float(M_PI),
        safe_stof(match[PITCH_MAX].str()) / 180.f * float(M_PI),
        safe_stof(match[DPITCH_MAX].str()) / 180.f * float(M_PI),
        safe_stof(match[YAW_LOCKED_ON_MAX].str()) / 180.f * float(M_PI),
        safe_stof(match[PITCH_LOCKED_ON_MAX].str()) / 180.f * float(M_PI),
        [rp]()mutable{return rp();},
        scene_config.physics_engine_config);
    follower->set_followed(followed_node, followed_rb);
    linker.link_relative_movable(yaw_node, follower);
    linker.link_relative_movable(pitch_node, follower->pitch_look_at_node());
}
