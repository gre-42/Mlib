#include "On_All_Users_Loaded_Level.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(content);
}

OnAllUsersLoadedLevel::OnAllUsersLoadedLevel(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void OnAllUsersLoadedLevel::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    auto l = args.arguments.at(KnownArgs::content);
    if (on_all_users_loaded_level_token.has_value()) {
        THROW_OR_ABORT("on_all_users_loaded_level_token already set");
    }
    on_all_users_loaded_level_token.emplace(
        remote_sites.on_all_users_loaded_level,
        [mle=args.macro_line_executor, content=args.arguments.at(KnownArgs::content)]()
        {
            mle(content, nullptr);
        });
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "on_all_users_loaded_level",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                OnAllUsersLoadedLevel(args.physics_scene()).execute(args);
            });
    }
} obj;

}
