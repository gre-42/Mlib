#include "Compute_Random_User_Ranks.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
}

void ComputeRandomUserRanks::execute(const LoadSceneJsonUserFunctionArgs &args) {
    args.arguments.validate(KnownArgs::options);
    if (args.local_json_macro_arguments == nullptr) {
        THROW_OR_ABORT("compute_random_user_ranks must be called from within a block");
    }
    auto& vars = *args.local_json_macro_arguments;
    args.remote_sites.compute_random_user_ranks();
    args.remote_sites.for_each_site_user(
        [&](UserInfo& user)
        {
            vars.set("random_rank_" + user.full_name, user.random_rank);
            vars.set("random_rank_str_" + user.full_name, std::to_string(user.random_rank));
            return true;
        }, UserTypes::ALL);
}

namespace {

struct RegisterJsonUserFunctionAll {
    RegisterJsonUserFunctionAll() {
        LoadSceneFuncs::register_json_user_function(
            "compute_random_user_ranks",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                ComputeRandomUserRanks::execute(args);
            });
    }
} obj;

}
