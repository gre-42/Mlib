#include "Create_Engine.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#ifndef WITHOUT_ALUT
#include <Mlib/Scene/Audio/Engine_Audio.hpp>
#endif
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
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
}

const std::string CreateEngine::key = "create_engine";

LoadSceneUserFunction CreateEngine::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    JsonMacroArguments json_macro_arguments{nlohmann::json::parse(args.line)};
    if (json_macro_arguments.contains_json(KnownArgs::audio)) {
        JsonMacroArguments c{json_macro_arguments.at(KnownArgs::audio)};
        c.validate(Audio::options);
        json_macro_arguments.insert_child(KnownArgs::audio, std::move(c));
    }
    CreateEngine(args.renderable_scene()).execute(json_macro_arguments, args);
};

CreateEngine::CreateEngine(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

float stow(float v) {
    return v * rpm;
}

float stop(float v) {
    return v * hp;
}

void CreateEngine::execute(
    const JsonMacroArguments& json_macro_arguments,
    const LoadSceneUserFunctionArgs& args)
{
    json_macro_arguments.validate(KnownArgs::options);
    auto rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(
        json_macro_arguments.at<std::string>(KnownArgs::rigid_body)).get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    EnginePower engine_power{
        Interp<float>{
            json_macro_arguments.at_vector<float>(KnownArgs::angular_vels, stow),
            json_macro_arguments.at_vector<float>(KnownArgs::powers, stop),
            OutOfRangeBehavior::CLAMP},
        json_macro_arguments.at<std::vector<float>>(KnownArgs::gear_ratios),
        json_macro_arguments.at<float>(KnownArgs::w_clutch) * rpm,
        json_macro_arguments.at<float>(KnownArgs::max_dw, INFINITY) * rpm / s};
    std::shared_ptr<EngineAudio> av;
    if (json_macro_arguments.contains_child(KnownArgs::audio)) {
        auto& a = json_macro_arguments.child(KnownArgs::audio);
        a.validate(Audio::options);
        av = std::make_shared<EngineAudio>(
            a.at("name"),
            paused,
            a.at<float>(Audio::p_idle) * hp,
            a.at<float>(Audio::p_reference) * hp);
    }
    auto ep = rb->engines_.try_emplace(
        json_macro_arguments.at<std::string>(KnownArgs::name),
        engine_power,
        json_macro_arguments.at<bool>(KnownArgs::hand_brake_pulled, false),
#ifdef WITHOUT_ALUT
        nullptr
#else
        av
#endif
        );
    if (!ep.second) {
        THROW_OR_ABORT("Engine with name \"" + json_macro_arguments.at<std::string>(KnownArgs::name) + "\" already exists");
    }
}
