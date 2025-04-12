#include "Set_Preferred_Car_Spawner.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Arguments.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(macro);
DECLARE_ARGUMENT(already_set_behavior);
}

namespace KnownLet {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(human_node_position);
DECLARE_ARGUMENT(human_node_angle_y);
DECLARE_ARGUMENT(car_node_position);
DECLARE_ARGUMENT(car_node_angles);
DECLARE_ARGUMENT(velocity);
DECLARE_ARGUMENT(angular_velocity);
DECLARE_ARGUMENT(suffix);
DECLARE_ARGUMENT(if_with_graphics);
DECLARE_ARGUMENT(if_with_physics);
}

SetPreferredCarSpawner::SetPreferredCarSpawner(RenderableScene& renderable_scene)
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void SetPreferredCarSpawner::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::string spawner_name = args.arguments.at<std::string>(KnownArgs::spawner);
    vehicle_spawners.get(spawner_name).set_spawn_vehicle(
        [macro_line_executor = args.macro_line_executor,
         spawner_name,
         asset_id = args.arguments.at<std::string>(KnownArgs::asset_id),
         macro = args.arguments.at(KnownArgs::macro),
         &snr = scene_node_resources,
         &cq = physics_engine.collision_query_,
         &scene = scene](const SpawnPoint& p, const SpawnArguments& a){
            auto dp = FixedArray<CompressedScenePos, 3>{
                (CompressedScenePos)0.f,
                a.y_offset,
                (CompressedScenePos)0.f
            };
            auto trafo = TransformationMatrix<float, ScenePos, 3>{
                tait_bryan_angles_2_matrix<float>(p.rotation),
                funpack(p.position + dp)};
            auto distancebox = snr.get_intersectables(asset_id + "_distancebox");
            if (!cq.can_spawn_at(trafo, distancebox)) {
                return false;
            }
            auto z = z3_from_3x3(trafo.R);
            nlohmann::json let{
                {KnownLet::human_node_position, trafo.t / (ScenePos)meters},
                {KnownLet::human_node_angle_y, std::atan2(z(0), z(2)) / degrees},
                {KnownLet::car_node_position, trafo.t / (ScenePos)meters},
                {KnownLet::car_node_angles, p.rotation / degrees},
                // Velocity and angular velocity should be calculated dynamically from the parent of the
                // spawn point using "parent.velocity_at_position" and "parent.angular_velocity_at_position".
                // Spawn points do not yet have a parent, so the values are set to zero here.
                {KnownLet::velocity, fixed_zeros<float, 3>() / kph},
                {KnownLet::angular_velocity, fixed_zeros<float, 3>() / rpm},  // this is not yet used in the scripts
                {KnownLet::suffix, a.suffix},
                {KnownLet::if_with_graphics, a.if_with_graphics},
                {KnownLet::if_with_physics, a.if_with_physics} };
            macro_line_executor.inserted_block_arguments(let)(macro, nullptr, nullptr);
            return true;
        },
        spawn_vehicle_already_set_behavior_from_string(
            args.arguments.at<std::string>(KnownArgs::already_set_behavior)));
}

namespace {

static struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_preferred_car_spawner",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                args.macro_line_executor.block_arguments().validate_complement(KnownLet::options);
                SetPreferredCarSpawner(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
