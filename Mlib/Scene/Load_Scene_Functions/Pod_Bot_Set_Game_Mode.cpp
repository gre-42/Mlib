#include "Pod_Bot_Set_Game_Mode.hpp"
#include <Mlib/Players/Mlib_Pod_Bot/Set_Pod_Bot_Game_Mode.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(GAME_MODE);

LoadSceneUserFunction PodBotSetGameMode::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_pod_bot_game_mode"
        "\\s+(as|cs|de|awp|aim|fy|es)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PodBotSetGameMode(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PodBotSetGameMode::PodBotSetGameMode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PodBotSetGameMode::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    set_pod_bot_game_mode(match[GAME_MODE].str());
}
