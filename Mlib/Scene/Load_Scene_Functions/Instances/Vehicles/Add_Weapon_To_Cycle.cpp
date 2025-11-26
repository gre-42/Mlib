#include "Add_Weapon_To_Cycle.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Weapon_Cycle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Bullets/Bullet_Property_Db.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
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
DECLARE_ARGUMENT(create_weapon);
DECLARE_ARGUMENT(create_externals);
}

AddWeaponToInventory::AddWeaponToInventory(
    PhysicsScene& physics_scene,
    const MacroLineExecutor& macro_line_executor) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene, &macro_line_executor }
{}

void AddWeaponToInventory::operator () (const JsonView& args) {
    args.validate(KnownArgs::options);

    DanglingBaseClassRef<SceneNode> cycle_node = scene.get_node(args.at<VariableAndHash<std::string>>(KnownArgs::cycle_node), DP_LOC);
    auto entry_name = args.at<std::string>(KnownArgs::entry_name);
    WeaponCycle& wc = get_weapon_cycle(cycle_node);
    auto ammo_type = args.at<InventoryItem>(KnownArgs::ammo_type);
    auto bullet_type = args.at<std::string>(KnownArgs::bullet_type);
    float cool_down = args.at<float>(KnownArgs::cool_down);

    std::function<void()> create_weapon;
    if (auto create = args.try_at(KnownArgs::create_weapon); create.has_value()) {
        create_weapon = [
            macro_line_executor = macro_line_executor,
            create = *create,
            ammo_type,
            bullet_type,
            cool_down]()
        {
            nlohmann::json let{
                { "ammo_type", ammo_type },
                { "bullet_type", bullet_type },
                { "cool_down", cool_down }};
            macro_line_executor.inserted_block_arguments(std::move(let))(create, nullptr);
        };
    }
    std::function<void(const VariableAndHash<std::string>& player_name)> create_externals;
    if (auto create = args.try_at(KnownArgs::create_externals); create.has_value()) {
        create_externals = [
            macro_line_executor = macro_line_executor,
            create = args.at(KnownArgs::create_externals),
            &players = players](const VariableAndHash<std::string>& player_name)
        {
            auto player = players.get_player(player_name, CURRENT_SOURCE_LOCATION);
            auto user_info = player->user_info();
            if (user_info == nullptr) {
                return;
            }
            nlohmann::json let{
                {"player_name", player_name},
                {"user_name", user_info->name},
                {"full_user_name", user_info->full_name}};
            if (user_info->type == UserType::LOCAL) {
                let["user_is_local"] = true;
                let["local_user_id"] = user_info->user_id;
            } else {
                let["user_is_local"] = false;
            }
            macro_line_executor.inserted_block_arguments(std::move(let))(create, nullptr);
        };
    }
    wc.add_weapon(
        entry_name,
        WeaponInfo{
            .create_weapon = std::move(create_weapon),
            .create_externals = std::move(create_externals),
            .ammo_type = ammo_type,
            .bullet_properties = bullet_property_db.get(args.at<VariableAndHash<std::string>>(KnownArgs::bullet_type)),
            .cool_down = cool_down * seconds,
            .range_min = args.at<float>(KnownArgs::range_min) * meters,
            .range_max = args.at<float>(KnownArgs::range_max) * meters});
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "add_weapon_to_cycle",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                AddWeaponToInventory(args.physics_scene(), args.macro_line_executor)(args.arguments);
            });
    }
} obj;

}
