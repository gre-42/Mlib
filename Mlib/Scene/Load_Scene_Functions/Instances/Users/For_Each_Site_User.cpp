#include "For_Each_Site_User.hpp"
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

ForEachSiteUser::ForEachSiteUser(UserType user_type)
    : user_type_{ user_type }
{}

ForEachSiteUser::~ForEachSiteUser() = default;

void ForEachSiteUser::execute(const LoadSceneJsonUserFunctionArgs &args) {
    args.arguments.validate(KnownArgs::options);
    auto l = args.arguments.at(KnownArgs::content);
    args.remote_sites.for_each_site_user(
        [&](UserInfo& user)
        {
            nlohmann::json let{
                {"user_name", user.name},
                {"full_user_name", user.full_name}
            };
            if (user.site_id.has_value()) {
                auto locals_site_id = args.remote_sites.get_local_site_id();
                if (!locals_site_id.has_value()) {
                    THROW_OR_ABORT("Local site ID not set");
                }
                if (*user.site_id == *locals_site_id) {
                    let["user_is_local"] = true;
                    let["local_user_id"] = user.user_id;
                } else {
                    let["user_is_local"] = false;
                    let["local_user_id"] = nlohmann::json::object();
                }
            } else {
                let["user_is_local"] = true;
                let["local_user_id"] = user.user_id;
            }
            if (args.local_json_macro_arguments != nullptr) {
                let.update(args.local_json_macro_arguments->json());
            }
            args.macro_line_executor.inserted_block_arguments(std::move(let))(l, nullptr);
        }, user_type_);
}

namespace {

struct RegisterJsonUserFunctionAll {
    RegisterJsonUserFunctionAll() {
        LoadSceneFuncs::register_json_user_function(
            "for_each_site_user",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ForEachSiteUser(UserType::ALL).execute(args);
            });
    }
} obj_all;

struct RegisterJsonUserFunctionLocal {
    RegisterJsonUserFunctionLocal() {
        LoadSceneFuncs::register_json_user_function(
            "for_each_user",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ForEachSiteUser(UserType::LOCAL).execute(args);
            });
    }
} obj_local;

}
