#include "Set_Objective.hpp"
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Game_Logic/Objective.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(objective);
}

SetObjective::SetObjective(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetObjective::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    if (game_logic == nullptr) {
        throw std::runtime_error("Scene has no game logic");
    }
    game_logic->team_deathmatch.set_objective(
        objective_from_string(args.arguments.at<std::string>(KnownArgs::objective)));
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "set_objective",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                SetObjective{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
