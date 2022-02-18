#include "Create_Engine.hpp"
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Audio/Engine_Audio.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

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

void CreateEngine::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto rb = dynamic_cast<RigidBodyVehicle*>(scene.get_node(match[1].str()).get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    EnginePower engine_power{
        Interp<float>{
            string_to_vector(match[3].str(), safe_stof),
            string_to_vector(match[4].str(), safe_stof),
            OutOfRangeBehavior::CLAMP},
        string_to_vector(match[5].str(), safe_stof)};
    auto ep = rb->engines_.insert({
        match[2].str(),
        RigidBodyEngine{
            engine_power,
            match[6].str().empty() ? false : safe_stob(match[6].str()),  // hand_brake_pulled
            match[7].matched ? std::make_shared<EngineAudio>(match[7].str()) : nullptr}});
    if (!ep.second) {
        throw std::runtime_error("Engine with name \"" + match[2].str() + "\" already exists");
    }
}
