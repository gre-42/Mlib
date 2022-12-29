#include "Create_Wheel.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Wheel.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
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
DECLARE_OPTION(NODE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(POSITION_Z);
DECLARE_OPTION(RADIUS);
DECLARE_OPTION(ENGINE);
DECLARE_OPTION(BREAK_FORCE);
DECLARE_OPTION(KS);
DECLARE_OPTION(KA);
DECLARE_OPTION(MUSF);
DECLARE_OPTION(MUSC);
DECLARE_OPTION(TIRE_ID);

LoadSceneUserFunction CreateWheel::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*wheel"
        "\\s+rigid_body=\\s*([\\w+-.]+)"
        "\\s+node=\\s*([\\w+-.]*)"
        "\\s+position=\\s*([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+radius=\\s*([\\w+-.]+)"
        "\\s+engine=\\s*([\\w+-.]+)"
        "\\s+brake_force=\\s*([\\w+-.]+)"
        "\\s+Ks=\\s*([\\w+-.]+)"
        "\\s+Ka=\\s*([\\w+-.]+)"
        "\\s+musF=\\s*([ \\w+-.]+)"
        "\\s+musC=\\s*([ \\w+-.]+)"
        "\\s+tire_id=\\s*(\\d+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateWheel(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateWheel::CreateWheel(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWheel::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string rigid_body = match[RIGID_BODY].str();
    std::string node = match[NODE].str();
    FixedArray<float, 3> position{
        safe_stof(match[POSITION_X].str()),
        safe_stof(match[POSITION_Y].str()),
        safe_stof(match[POSITION_Z].str())};
    float radius = safe_stof(match[RADIUS].str()) * meters;
    std::string engine = match[ENGINE].str();
    float brake_force = safe_stof(match[BREAK_FORCE].str()) * N;
    float Ks = safe_stof(match[KS].str()) * N;
    float Ka = safe_stof(match[KA].str()) * N * s;
    Interp<float> mus{
        string_to_vector(match[MUSF].str(), safe_stof),
        string_to_vector(match[MUSC].str(), safe_stof),
        OutOfRangeBehavior::CLAMP};
    size_t tire_id = safe_stoi(match[TIRE_ID].str());

    auto rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(rigid_body).get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    if (!node.empty()) {
        std::shared_ptr<Wheel> wheel = std::make_shared<Wheel>(
            *rb,
            physics_engine.advance_times_,
            tire_id,
            radius);
        Linker{ physics_engine.advance_times_ }.link_relative_movable(scene.get_node(node), wheel);
    }
    {
        // From: https://www.nanolounge.de/21977/federkonstante-und-masse-bei-auto
        // Ds = 1000 / 4 * 9.8 / 0.02 = 122500 = 1.225e5
        // Da * 1 = 1000 / 4 * 9.8 => Da = 1e4 / 4
        auto tp = rb->tires_.insert({
            tire_id,
            Tire{
                engine,
                brake_force,
                Ks,
                Ka,
                mus,
                CombinedMagicFormula<float>{
                    .f = FixedArray<MagicFormulaArgmax<float>, 2>{
                        MagicFormulaArgmax<float>{MagicFormula<float>{.B = 41.f * 0.044f * scene_config.physics_engine_config.longitudinal_friction_steepness}},
                        MagicFormulaArgmax<float>{MagicFormula<float>{.B = 41.f * 0.044f * scene_config.physics_engine_config.lateral_friction_steepness}}
                    }
                },
                position,
                radius}});
        if (!tp.second) {
            THROW_OR_ABORT("Tire with ID \"" + std::to_string(tire_id) + "\" already exists");
        }
    }
}
