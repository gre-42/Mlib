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
        [&](std::optional<RemoteSiteId> site_id, uint32_t user_id, UserInfo& user)
        {
            if (site_id.has_value()) {
                nlohmann::json let{
                    {"user_id", user_id},
                    {"user_name", std::to_string(user_id)},
                    {"full_user_name", std::to_string(*site_id) + '_' + std::to_string(user_id)}
                };
                if (args.local_json_macro_arguments != nullptr) {
                    let.update(args.local_json_macro_arguments->json());
                }
                args.macro_line_executor.inserted_block_arguments(std::move(let))(l, nullptr);
            } else {
                nlohmann::json let{
                    {"user_id", user_id},
                    {"user_name", std::to_string(user_id)},
                    {"full_user_name", std::to_string(user_id)}
                };
                if (args.local_json_macro_arguments != nullptr) {
                    let.update(args.local_json_macro_arguments->json());
                }
                args.macro_line_executor.inserted_block_arguments(std::move(let))(l, nullptr);
            }
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
