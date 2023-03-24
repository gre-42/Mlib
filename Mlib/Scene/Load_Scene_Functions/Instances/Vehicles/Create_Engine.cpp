#include "Create_Engine.hpp"
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Regex_Select.hpp>
#ifndef WITHOUT_ALUT
#include <Mlib/Scene/Audio/Engine_Audio.hpp>
#endif
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(RIGID_BODY);
DECLARE_OPTION(NAME);
DECLARE_OPTION(ANGULAR_VELS);
DECLARE_OPTION(POWERS);
DECLARE_OPTION(GEAR_RATIOS);
DECLARE_OPTION(MAX_DW);
DECLARE_OPTION(HAND_BRAKE_PULLED);
DECLARE_OPTION(AUDIO);

LoadSceneUserFunction CreateEngine::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*create_engine"
        "\\s+rigid_body=([\\w+-.]+)"
        "\\s+name=([\\w+-.]+)"
        "\\s+angular_vels=([ \\w+-.]+)"
        "\\s+powers=([ \\w+-.]+)"
        "\\s+gear_ratios=([ \\w+-.]+)"
        "\\s+max_dw=([\\w+-.]+)"
        "(?:\\s+hand_brake_pulled=(0|1))?"
        "(?:\\s+audio=([\\w+-. \\(\\)/]+))?");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateEngine(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateEngine::CreateEngine(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

float stow(const std::string& str) {
    return safe_stof(str) * rpm;
}

float stop(const std::string& str) {
    return safe_stof(str) * hp;
}

void CreateEngine::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(match[RIGID_BODY].str()).get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    EnginePower engine_power{
        Interp<float>{
            string_to_vector(match[ANGULAR_VELS].str(), stow),
            string_to_vector(match[POWERS].str(), stop),
            OutOfRangeBehavior::CLAMP},
        string_to_vector(match[GEAR_RATIOS].str(), safe_stof),
        match[MAX_DW].matched
            ? safe_stof(match[MAX_DW].str()) * rpm / s
            : INFINITY};
    auto ep = rb->engines_.try_emplace(
        match[NAME].str(),
        engine_power,
        match[HAND_BRAKE_PULLED].str().empty() ? false : safe_stob(match[HAND_BRAKE_PULLED].str()),
#ifdef WITHOUT_ALUT
        nullptr);
#else
        match[AUDIO].matched ? std::make_shared<EngineAudio>(match[AUDIO].str(), paused) : nullptr);
#endif
    if (!ep.second) {
        THROW_OR_ABORT("Engine with name \"" + match[2].str() + "\" already exists");
    }
}
