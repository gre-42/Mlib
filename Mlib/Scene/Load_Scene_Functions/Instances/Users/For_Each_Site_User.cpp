#include "For_Each_Site_User.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

static void visit_user(
    const MacroLineExecutor& macro_line_executor,
    RemoteSites& remote_sites,
    JsonMacroArguments* local_json_macro_arguments,
    const nlohmann::json& command,
    const UserInfo& user)
{
    nlohmann::json let{
        {"user_name", user.name},
        {"full_user_name", user.full_name},
        {"user_is_local", (user.type == UserType::LOCAL)}
    };
    if (user.site_id.has_value()) {
        auto locals_site_id = remote_sites.get_local_site_id();
        if (!locals_site_id.has_value()) {
            THROW_OR_ABORT("Local site ID not set");
        }
        if (*user.site_id == *locals_site_id) {
            let["local_user_id"] = user.user_id;
        } else {
            let["local_user_id"] = nlohmann::json::object();
        }
    } else {
        let["local_user_id"] = user.user_id;
    }
    if (local_json_macro_arguments != nullptr) {
        let.update(local_json_macro_arguments->json());
    }
    macro_line_executor.inserted_block_arguments(std::move(let))(command, nullptr);
};

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(content);
}

ForEachSiteUser::ForEachSiteUser(UserTypes user_types)
    : user_types_{ user_types }
{}

ForEachSiteUser::~ForEachSiteUser() = default;

void ForEachSiteUser::execute(const LoadSceneJsonUserFunctionArgs &args) {
    args.arguments.validate(KnownArgs::options);
    auto content = args.arguments.at(KnownArgs::content);
    args.remote_sites.for_each_site_user(
        [&](const UserInfo& user)
        {
            visit_user(args.macro_line_executor, args.remote_sites, args.local_json_macro_arguments, content, user);
            return true;
        }, user_types_);
}


OnUserLoadedLevel::OnUserLoadedLevel(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

OnUserLoadedLevel::~OnUserLoadedLevel() = default;

void OnUserLoadedLevel::execute(const LoadSceneJsonUserFunctionArgs &args) {
    args.arguments.validate(KnownArgs::options);
    if (on_user_loaded_level_token.has_value()) {
        THROW_OR_ABORT("on_user_loaded_level_token already set");
    }
    on_user_loaded_level_token.emplace(
        args.remote_sites.on_user_loaded_level,
        [mle = args.macro_line_executor,
         content = args.arguments.at(KnownArgs::content),
         &remote_sites = args.remote_sites](const UserInfo& user)
        {
            visit_user(mle, remote_sites, nullptr, content, user);
            return true;
        });
}

namespace {

struct RegisterJsonUserFunctionAll {
    RegisterJsonUserFunctionAll() {
        LoadSceneFuncs::register_json_user_function(
            "for_each_site_user",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ForEachSiteUser(UserTypes::ALL).execute(args);
            });
    }
} obj_all;

struct RegisterJsonUserFunctionLocal {
    RegisterJsonUserFunctionLocal() {
        LoadSceneFuncs::register_json_user_function(
            "for_each_user",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ForEachSiteUser(UserTypes::ALL_LOCAL).execute(args);
            });
    }
} obj_local;

struct RegisterJsonUserFunctionOnLoadedLevel {
    RegisterJsonUserFunctionOnLoadedLevel() {
        LoadSceneFuncs::register_json_user_function(
            "on_site_user_loaded_level",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                OnUserLoadedLevel(args.physics_scene()).execute(args);
            });
    }
} obj_on_loaded_level;

}
