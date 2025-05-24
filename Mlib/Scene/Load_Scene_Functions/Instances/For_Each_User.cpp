#include "For_Each_User.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Players/Containers/Users.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(content);
}

ForEachUser::ForEachUser(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void ForEachUser::execute(const LoadSceneJsonUserFunctionArgs &args) {
    users.for_each_user(
        [l = args.arguments.at(KnownArgs::content),
         mle = args.macro_line_executor](uint32_t i)
        {
            nlohmann::json let{
                {"user_id", i},
                {"user_name", std::to_string(i)}
            };
            mle.inserted_block_arguments(let)(l, nullptr, nullptr);
        }
    );
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "for_each_user",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                ForEachUser(args.physics_scene()).execute(args);
            });
    }
} obj;

}
