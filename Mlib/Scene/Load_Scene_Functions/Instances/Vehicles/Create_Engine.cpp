#include "Create_Engine.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#ifndef WITHOUT_ALUT
#include <Mlib/Scene/Audio/Engine_Audio.hpp>
#endif
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(rigid_body);
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(angular_vels);
DECLARE_ARGUMENT(powers);
DECLARE_ARGUMENT(gear_ratios);
DECLARE_ARGUMENT(w_clutch);
DECLARE_ARGUMENT(max_dw);
DECLARE_ARGUMENT(hand_brake_pulled);
DECLARE_ARGUMENT(audio);
}

namespace Audio {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(p_idle);
DECLARE_ARGUMENT(p_reference);
DECLARE_ARGUMENT(mute);
}

const std::string CreateEngine::key = "create_engine";

LoadSceneJsonUserFunction CreateEngine::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    if (args.arguments.contains(KnownArgs::audio)) {
        args.arguments.child(KnownArgs::audio).validate(Audio::options);
    }
    CreateEngine(args.renderable_scene()).execute(args);
};

CreateEngine::CreateEngine(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

static inline float stow(float v) {
    return v * rpm;
}

static inline float stop(float v) {
    return v * hp;
}

void CreateEngine::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::rigid_body), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    std::optional<EnginePower> engine_power;
    if (args.arguments.contains(KnownArgs::angular_vels) ||
        args.arguments.contains(KnownArgs::powers) ||
        args.arguments.contains(KnownArgs::gear_ratios) ||
        args.arguments.contains(KnownArgs::w_clutch) ||
        args.arguments.contains(KnownArgs::max_dw))
    {
        engine_power = EnginePower{
            Interp<float>{
                args.arguments.at_vector<float>(KnownArgs::angular_vels, stow),
                args.arguments.at_vector<float>(KnownArgs::powers, stop),
                OutOfRangeBehavior::CLAMP},
            args.arguments.at<std::vector<float>>(KnownArgs::gear_ratios),
            args.arguments.at<float>(KnownArgs::w_clutch) * rpm,
            args.arguments.at<float>(KnownArgs::max_dw, INFINITY) * rpm / seconds};
    }
#ifndef WITHOUT_ALUT
    std::shared_ptr<EngineAudio> av;
    if (args.arguments.contains(KnownArgs::audio)) {
        auto a = args.arguments.child(KnownArgs::audio);
        a.validate(Audio::options);
        if (!a.at<bool>(Audio::mute)) {
            av = std::make_shared<EngineAudio>(
                a.at<std::string>(Audio::name),
                paused,
                paused_changed,
                a.at<float>(Audio::p_idle) * hp,
                a.at<float>(Audio::p_reference) * hp);
        }
    }
#endif
    rb.engines_.add(
        args.arguments.at<VariableAndHash<std::string>>(KnownArgs::name),
        engine_power,
        args.arguments.at<bool>(KnownArgs::hand_brake_pulled, false),
#ifdef WITHOUT_ALUT
        nullptr
#else
        av
#endif
        );
}
