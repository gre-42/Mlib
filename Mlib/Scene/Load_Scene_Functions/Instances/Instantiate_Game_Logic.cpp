#include "Instantiate_Game_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(setup_new_round);
}

const std::string InstantiateGameLogic::key = "instantiate_game_logic";

LoadSceneJsonUserFunction InstantiateGameLogic::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    InstantiateGameLogic(args.renderable_scene()).execute(args);
};

InstantiateGameLogic::InstantiateGameLogic(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void InstantiateGameLogic::execute(const LoadSceneJsonUserFunctionArgs &args) {
    renderable_scene.instantiate_game_logic(
        [l = args.arguments.at(KnownArgs::setup_new_round),
         mle = args.macro_line_executor]()
        {
            mle(l, nullptr, nullptr);
        }
    );
}
