#include "Create_Engine.hpp"
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#ifndef WITHOUT_ALUT
#include <Mlib/Scene/Audio/Engine_Audio.hpp>
#endif
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(RIGID_BODY);
DECLARE_OPTION(NAME);
DECLARE_OPTION(ANGULAR_VELS);
DECLARE_OPTION(POWERS);
DECLARE_OPTION(GEAR_RATIOS);
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
        "(?:\\s+hand_brake_pulled=(0|1))?"
        "(?:\\s+audio=([\\w-. \\(\\)/+-]+))?");
    std::smatch match;
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
    return safe_stof(str) * radians / s;
}

float stop(const std::string& str) {
    return safe_stof(str) * W;
}

void CreateEngine::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto rb = dynamic_cast<RigidBodyVehicle*>(scene.get_node(match[RIGID_BODY].str()).get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    EnginePower engine_power{
        Interp<float>{
            string_to_vector(match[ANGULAR_VELS].str(), stow),
            string_to_vector(match[POWERS].str(), stop),
            OutOfRangeBehavior::CLAMP},
        string_to_vector(match[GEAR_RATIOS].str(), safe_stof)};
    auto ep = rb->engines_.insert({
        match[NAME].str(),
        RigidBodyEngine{
            engine_power,
            match[HAND_BRAKE_PULLED].str().empty() ? false : safe_stob(match[HAND_BRAKE_PULLED].str()),
            match[AUDIO].matched ? std::make_shared<EngineAudio>(match[AUDIO].str(), audio_paused) : nullptr}});
    if (!ep.second) {
        throw std::runtime_error("Engine with name \"" + match[2].str() + "\" already exists");
    }
}
