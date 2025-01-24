#include "Set_Preferred_Car_Spawner.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(spawner);
DECLARE_ARGUMENT(macro);
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

const std::string SetPreferredCarSpawner::key = "set_preferred_car_spawner";

LoadSceneJsonUserFunction SetPreferredCarSpawner::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    args.macro_line_executor.block_arguments().validate_complement(KnownLet::options);
    SetPreferredCarSpawner(args.renderable_scene()).execute(args);
};

SetPreferredCarSpawner::SetPreferredCarSpawner(RenderableScene& renderable_scene)
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void SetPreferredCarSpawner::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::string spawner_name = args.arguments.at<std::string>(KnownArgs::spawner);
    auto macro = args.arguments.at(KnownArgs::macro);
    vehicle_spawners.get(spawner_name).set_spawn_vehicle(
        [macro_line_executor = args.macro_line_executor,
         spawner_name,
         macro,
         &scene = scene](const SpawnPoint& p){
            auto z = z3_from_3x3(tait_bryan_angles_2_matrix(p.rotation));
            nlohmann::json let{
                {KnownLet::human_node_position, funpack(p.position) / (ScenePos)meters},
                {KnownLet::human_node_angle_y, std::atan2(z(0), z(2)) / degrees},
                {KnownLet::car_node_position, funpack(p.position) / (ScenePos)meters},
                {KnownLet::car_node_angles, p.rotation / degrees},
                // Velocity and angular velocity should be calculated dynamically from the parent of the
                // spawn point using "parent.velocity_at_position" and "parent.angular_velocity_at_position".
                // Spawn points do not yet have a parent, so the values are set to zero here.
                {KnownLet::velocity, fixed_zeros<float, 3>() / kph},
                {KnownLet::angular_velocity, fixed_zeros<float, 3>() / rpm},  // this is not yet used in the scripts
                {KnownLet::suffix, "_" + spawner_name + scene.get_temporary_instance_suffix()},
                {KnownLet::if_with_graphics, true},
                {KnownLet::if_with_physics, true} };
            macro_line_executor.inserted_block_arguments(let)(macro, nullptr, nullptr);
        }
    );
}
