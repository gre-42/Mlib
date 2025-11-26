#include "Create_Player.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Advance_Times/Player_Site_Privileges.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Players/User_Account/User_Account.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Player.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstdint>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(name);
DECLARE_ARGUMENT(team);
DECLARE_ARGUMENT(full_user_name);
DECLARE_ARGUMENT(user_account_key);
DECLARE_ARGUMENT(game_mode);
DECLARE_ARGUMENT(player_role);
DECLARE_ARGUMENT(unstuck_mode);
DECLARE_ARGUMENT(behavior);
DECLARE_ARGUMENT(driving_direction);
}

CreatePlayer::CreatePlayer(
    PhysicsScene& physics_scene,
    const MacroLineExecutor& macro_line_executor) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene, &macro_line_executor }
{}

void CreatePlayer::execute(const JsonView& args, PlayerCreator creator)
{
    args.validate(KnownArgs::options);
    if (game_logic == nullptr) {
        THROW_OR_ABORT("Game logic is null, cannot create player");
    }
    std::shared_ptr<UserAccount> user_account;
    if (auto user_account_key = args.try_at_non_null<std::string>(KnownArgs::user_account_key);
        user_account_key.has_value())
    {
        user_account = std::make_shared<UserAccount>(macro_line_executor, *user_account_key);
    }
    DanglingBaseClassPtr<const UserInfo> user_info = nullptr;
    if (auto full_user_name = args.try_at<VariableAndHash<std::string>>(KnownArgs::full_user_name);
        full_user_name.has_value())
    {
        user_info = remote_sites.get_user(*full_user_name).ptr().set_loc(CURRENT_SOURCE_LOCATION);
    }
    auto name = args.at<VariableAndHash<std::string>>(KnownArgs::name);
    auto site_privileges = PlayerSitePrivileges::NONE;
    if (remote_scene != nullptr) {
        if ((user_info != nullptr) && (user_info->site_id == remote_scene->local_site_id())) {
            site_privileges |= PlayerSitePrivileges::CONTROLLER;
        }
        if (creator == PlayerCreator::LOCAL) {
            site_privileges |= PlayerSitePrivileges::MANAGER;
        }
    } else {
        site_privileges |= PlayerSitePrivileges::CONTROLLER;
        site_privileges |= PlayerSitePrivileges::MANAGER;
    }
    auto player = global_object_pool.create_unique<Player>(
        CURRENT_SOURCE_LOCATION,
        scene,
        supply_depots,
        game_logic->navigate,
        game_logic->supply_depots_waypoints_collection,
        game_logic->spawner,
        scene_config.physics_engine_config,
        physics_engine.collision_query_,
        vehicle_spawners,
        players,
        site_privileges,
        user_info,
        name,
        args.at<std::string>(KnownArgs::team),
        std::move(user_account),
        game_mode_from_string(args.at<std::string>(KnownArgs::game_mode)),
        player_role_from_string(args.at<std::string>(KnownArgs::player_role)),
        unstuck_mode_from_string(args.at<std::string>(KnownArgs::unstuck_mode)),
        args.at<std::string>(KnownArgs::behavior),
        driving_direction_from_string(args.at<std::string>(KnownArgs::driving_direction)),
        delete_node_mutex,
        countdown_start);
    players.add_player({ *player, CURRENT_SOURCE_LOCATION });
    physics_engine.advance_times_.add_advance_time({ *player, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    physics_engine.add_external_force_provider(*player);
    if ((remote_scene != nullptr) && (creator == PlayerCreator::LOCAL)) {
        remote_scene->create_local<RemotePlayer>(
            CURRENT_SOURCE_LOCATION,
            DanglingBaseClassRef<Player>{*player, CURRENT_SOURCE_LOCATION},
            DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION});
    }
    player.release();
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "player_create",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreatePlayer(args.physics_scene(), args.macro_line_executor).execute(args.arguments, PlayerCreator::LOCAL);
            });
    }
} obj;

}
