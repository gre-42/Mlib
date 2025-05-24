#include "Add_Weapon_To_Cycle.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Weapon_Cycle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(cycle_node);
DECLARE_ARGUMENT(entry_name);
DECLARE_ARGUMENT(ammo_type);
DECLARE_ARGUMENT(bullet_type);
DECLARE_ARGUMENT(cool_down);
DECLARE_ARGUMENT(range_min);
DECLARE_ARGUMENT(range_max);
DECLARE_ARGUMENT(create);
}

const std::string AddWeaponToInventory::key = "add_weapon_to_cycle";

LoadSceneJsonUserFunction AddWeaponToInventory::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    AddWeaponToInventory(args.physics_scene()).execute(args);
};

AddWeaponToInventory::AddWeaponToInventory(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void AddWeaponToInventory::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> cycle_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::cycle_node), DP_LOC);
    std::string entry_name = args.arguments.at<std::string>(KnownArgs::entry_name);
    auto create = args.arguments.at(KnownArgs::create);
    WeaponCycle& wc = get_weapon_cycle(cycle_node);
    std::string ammo_type = args.arguments.at<std::string>(KnownArgs::ammo_type);
    std::string bullet_type = args.arguments.at<std::string>(KnownArgs::bullet_type);
    float cool_down = args.arguments.at<float>(KnownArgs::cool_down);
    wc.add_weapon(
        entry_name,
        WeaponInfo{
            .create_weapon = [
                macro_line_executor = args.macro_line_executor,
                create,
                ammo_type,
                bullet_type,
                cool_down](const std::optional<std::string>& player_name)
            {
                nlohmann::json let{
                    { "WEAPON_DEMANDER", player_name.has_value() ? nlohmann::json(*player_name) : nlohmann::json()},
                    { "AMMO_TYPE", ammo_type },
                    { "BULLET_TYPE", bullet_type },
                    { "COOL_DOWN", cool_down }};
                macro_line_executor.inserted_block_arguments(let)(create, nullptr, nullptr);
            },
            .ammo_type = ammo_type,
            .bullet_properties = args.bullet_property_db.get(args.arguments.at<std::string>(KnownArgs::bullet_type)),
            .cool_down = cool_down * seconds,
            .range_min = args.arguments.at<float>(KnownArgs::range_min) * meters,
            .range_max = args.arguments.at<float>(KnownArgs::range_max) * meters});
}
