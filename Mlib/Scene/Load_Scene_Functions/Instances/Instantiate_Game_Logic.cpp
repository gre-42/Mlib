#include "Instantiate_Game_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(setup_new_round);
}

InstantiateGameLogic::InstantiateGameLogic(PhysicsScene& physics_scene) 
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void InstantiateGameLogic::execute(const LoadSceneJsonUserFunctionArgs &args) {
    physics_scene.instantiate_game_logic(
        [l = args.arguments.at(KnownArgs::setup_new_round),
         mle = args.macro_line_executor]()
        {
            mle(l, nullptr);
        }
    );
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "instantiate_game_logic",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                InstantiateGameLogic(args.physics_scene()).execute(args);
            });
    }
} obj;

}
