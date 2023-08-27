#include "Create_Wheel.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Wheel.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(rigid_body);
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(position);
DECLARE_ARGUMENT(radius);
DECLARE_ARGUMENT(engine);
DECLARE_ARGUMENT(delta_engine);
DECLARE_ARGUMENT(brake_force);
DECLARE_ARGUMENT(Ks);
DECLARE_ARGUMENT(Ka);
DECLARE_ARGUMENT(musF);
DECLARE_ARGUMENT(musC);
DECLARE_ARGUMENT(tire_id);
}

const std::string CreateWheel::key = "wheel";

LoadSceneJsonUserFunction CreateWheel::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateWheel(args.renderable_scene()).execute(args);
};

CreateWheel::CreateWheel(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWheel::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::string rigid_body = args.arguments.at<std::string>(KnownArgs::rigid_body);
    std::string node = args.arguments.at<std::string>(KnownArgs::node);
    auto position = args.arguments.at<FixedArray<float, 3>>(KnownArgs::position);;
    float radius = args.arguments.at<float>(KnownArgs::radius) * meters;
    auto engine = args.arguments.at<std::string>(KnownArgs::engine);
    auto delta_engine = args.arguments.try_at<std::string>(KnownArgs::delta_engine);
    float brake_force = args.arguments.at<float>(KnownArgs::brake_force) * N;
    float Ks = args.arguments.at<float>(KnownArgs::Ks) * N;
    float Ka = args.arguments.at<float>(KnownArgs::Ka) * N * s;
    Interp<float> mus{
        args.arguments.at<std::vector<float>>(KnownArgs::musF),
        args.arguments.at<std::vector<float>>(KnownArgs::musC),
        OutOfRangeBehavior::CLAMP};
    size_t tire_id = args.arguments.at<size_t>(KnownArgs::tire_id);

    auto rb = dynamic_cast<RigidBodyVehicle*>(&scene.get_node(rigid_body)->get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    if (!node.empty()) {
        auto wheel = std::make_unique<Wheel>(
            *rb,
            physics_engine.advance_times_,
            tire_id,
            radius);
        Linker{ physics_engine.advance_times_ }.link_relative_movable(scene.get_node(node), std::move(wheel));
    }
    {
        // From: https://www.nanolounge.de/21977/federkonstante-und-masse-bei-auto
        // Ds = 1000 / 4 * 9.8 / 0.02 = 122500 = 1.225e5
        // Da * 1 = 1000 / 4 * 9.8 => Da = 1e4 / 4
        auto tp = rb->tires_.insert({
            tire_id,
            Tire{
                engine,
                delta_engine,
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
