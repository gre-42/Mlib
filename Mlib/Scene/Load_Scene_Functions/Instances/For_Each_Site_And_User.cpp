#include "For_Each_Site_And_User.hpp"
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

void ForEachSiteAndUser::execute(const LoadSceneJsonUserFunctionArgs &args) {
    args.arguments.validate(KnownArgs::options);
    args.remote_sites.for_each_site_user(
        [l = args.arguments.at(KnownArgs::content),
         mle = args.macro_line_executor](RemoteSiteId site_id, uint32_t user_id)
        {
            nlohmann::json let{
                {"site_id", site_id},
                {"user_id", user_id},
                {"site_name", std::to_string(site_id)},
                {"user_name", std::to_string(user_id)}
            };
            mle.inserted_block_arguments(let)(l, nullptr);
        }
    );
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "for_each_site_and_user",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ForEachSiteAndUser::execute(args);
            });
    }
} obj;

}
