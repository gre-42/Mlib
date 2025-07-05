#include "Create_Wheel.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Actuators/Tire.hpp>
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
DECLARE_ARGUMENT(vehicle);
DECLARE_ARGUMENT(wheel);
DECLARE_ARGUMENT(vehicle_mount_0);
DECLARE_ARGUMENT(vehicle_mount_1);
DECLARE_ARGUMENT(radius);
DECLARE_ARGUMENT(engine);
DECLARE_ARGUMENT(delta_engine);
DECLARE_ARGUMENT(brake_force);
DECLARE_ARGUMENT(brake_torque);
DECLARE_ARGUMENT(Ks);
DECLARE_ARGUMENT(Ka);
DECLARE_ARGUMENT(Ke);
DECLARE_ARGUMENT(musF);
DECLARE_ARGUMENT(musC);
DECLARE_ARGUMENT(tire_id);
}

namespace WheelArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(vehicle_mount_0);
DECLARE_ARGUMENT(vehicle_mount_1);
}

const std::string CreateWheel::key = "wheel";

LoadSceneJsonUserFunction CreateWheel::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateWheel(args.physics_scene()).execute(args);
};

CreateWheel::CreateWheel(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void CreateWheel::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto vehicle = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::vehicle);
    auto wheel_node_name = args.arguments.try_at_non_null<VariableAndHash<std::string>>(KnownArgs::wheel);
    float radius = args.arguments.at<float>(KnownArgs::radius) * meters;
    auto engine = args.arguments.at<VariableAndHash<std::string>>(KnownArgs::engine);
    auto delta_engine = args.arguments.try_at<VariableAndHash<std::string>>(KnownArgs::delta_engine);
    Interp<float> mus{
        args.arguments.at<std::vector<float>>(KnownArgs::musF),
        args.arguments.at<std::vector<float>>(KnownArgs::musC),
        OutOfRangeBehavior::CLAMP};
    size_t tire_id = args.arguments.at<size_t>(KnownArgs::tire_id);

    auto& rb = get_rigid_body_vehicle(scene.get_node(vehicle, DP_LOC));
    RigidBodyPulses* wheel_rbp = nullptr;
    if (wheel_node_name.has_value()) {
        auto wheel_node = scene.get_node(*wheel_node_name, DP_LOC);
        if (has_rigid_body_vehicle(wheel_node)) {
            wheel_rbp = &get_rigid_body_vehicle(wheel_node).rbp_;
        } else {
            auto wheel = std::make_unique<Wheel>(
                rb,
                physics_engine.advance_times_,
                tire_id,
                radius);
            Linker{ physics_engine.advance_times_ }.link_relative_movable<Wheel>(
                wheel_node,
                { *wheel, CURRENT_SOURCE_LOCATION },
                CURRENT_SOURCE_LOCATION);
            global_object_pool.add(std::move(wheel), CURRENT_SOURCE_LOCATION);
        }
    }
    {
        // From: https://www.nanolounge.de/21977/federkonstante-und-masse-bei-auto
        // Ds = 1000 / 4 * 9.8 / 0.02 = 122500 = 1.225e5
        // Da * 1 = 1000 / 4 * 9.8 => Da = 1e4 / 4
        rb.tires_.add(
            tire_id,
            engine,
            std::move(delta_engine),
            wheel_rbp,
            args.arguments.at<float>(KnownArgs::brake_force) * N,
            args.arguments.at<float>(KnownArgs::brake_torque) * N * meters,
            args.arguments.at<float>(KnownArgs::Ks) * N,
            args.arguments.at<float>(KnownArgs::Ka) * N / (meters / seconds),
            args.arguments.at<float>(KnownArgs::Ke),
            mus,
            CombinedMagicFormula<float>{
                .f = FixedArray<MagicFormulaArgmax<float>, 2>{
                    MagicFormulaArgmax<float>{MagicFormula<float>{.B = 41.f * 0.044f * scene_config.physics_engine_config.longitudinal_friction_steepness}},
                    MagicFormulaArgmax<float>{MagicFormula<float>{.B = 41.f * 0.044f * scene_config.physics_engine_config.lateral_friction_steepness}}
                }
            },
            args.arguments.at<EFixedArray<float, 3>>(KnownArgs::vehicle_mount_0),
            args.arguments.at<EFixedArray<float, 3>>(KnownArgs::vehicle_mount_1),
            radius);
    }
}
