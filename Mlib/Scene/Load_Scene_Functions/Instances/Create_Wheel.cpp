#include "Create_Wheel.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Wheel.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

LoadSceneUserFunction CreateWheel::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*wheel"
        "\\s+rigid_body=([\\w+-.]+)"
        "\\s+node=([\\w+-.]*)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+radius=([\\w+-.]+)"
        "\\s+engine=([\\w+-.]+)"
        "\\s+break_force=([\\w+-.]+)"
        "\\s+sKs=([\\w+-.]+)"
        "\\s+sKa=([\\w+-.]+)"
        "\\s+pKs=([\\w+-.]+)"
        "\\s+pKa=([\\w+-.]+)"
        "\\s+musF=([ \\w+-.]+)"
        "\\s+musC=([ \\w+-.]+)"
        "\\s+mufF=([ \\w+-.]+)"
        "\\s+mufC=([ \\w+-.]+)"
        "\\s+tire_id=(\\d+)$");
    std::smatch match;
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
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string rigid_body = match[1].str();
    std::string node = match[2].str();
    FixedArray<float, 3> position{
        safe_stof(match[3].str()),
        safe_stof(match[4].str()),
        safe_stof(match[5].str())};
    float radius = safe_stof(match[6].str());
    std::string engine = match[7].str();
    float break_force = safe_stof(match[8].str());
    float sKs = safe_stof(match[9].str());
    float sKa = safe_stof(match[10].str());
    float pKs = safe_stof(match[11].str());
    float pKa = safe_stof(match[12].str());
    Interp<float> mus{string_to_vector(match[13].str(), safe_stof), string_to_vector(match[14].str(), safe_stof), OutOfRangeBehavior::CLAMP};
    Interp<float> muk{string_to_vector(match[15].str(), safe_stof), string_to_vector(match[16].str(), safe_stof), OutOfRangeBehavior::CLAMP};
    size_t tire_id = safe_stoi(match[17].str());

    auto rb = dynamic_cast<RigidBodyVehicle*>(scene.get_node(rigid_body).get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    if (!node.empty()) {
        std::shared_ptr<Wheel> wheel = std::make_shared<Wheel>(
            *rb,
            physics_engine.advance_times_,
            tire_id,
            radius,
            scene_config.physics_engine_config.physics_type,
            scene_config.physics_engine_config.resolve_collision_type);
        Linker{ physics_engine.advance_times_ }.link_relative_movable(scene.get_node(node), wheel);
    }
    {
        // From: https://www.nanolounge.de/21977/federkonstante-und-masse-bei-auto
        // Ds = 1000 / 4 * 9.8 / 0.02 = 122500 = 1.225e5

        // Da * 1 = 1000 / 4 * 9.8 => Da = 1e4 / 4
        size_t nsprings_tracking = 1;
        float max_dist = 0.3f;
        auto tp = rb->tires_.insert({
            tire_id,
            Tire{
                engine,
                break_force,
                sKs,
                sKa,
                mus,
                muk,
                ShockAbsorber{pKs, pKa},
                TrackingWheel{
                    {1.f, 0.f, 0.f},
                    radius,
                    nsprings_tracking,
                    max_dist,
                    scene_config.physics_engine_config.dt / scene_config.physics_engine_config.oversampling},
                CombinedMagicFormula<float>{
                    .f = FixedArray<MagicFormulaArgmax<float>, 2>{
                        MagicFormulaArgmax<float>{MagicFormula<float>{.B = 41.f * 0.044f * scene_config.physics_engine_config.longitudinal_friction_steepness}},
                        MagicFormulaArgmax<float>{MagicFormula<float>{.B = 41.f * 0.044f * scene_config.physics_engine_config.lateral_friction_steepness}}
                    }
                },
                position,
                radius}});
        if (!tp.second) {
            throw std::runtime_error("Tire with ID \"" + std::to_string(tire_id) + "\" already exists");
        }
    }
}
