#include "Set_Vip.hpp"
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>

using namespace Mlib;

LoadSceneUserFunction SetVip::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_vip player=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetVip(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetVip::SetVip(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetVip::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    game_logic.bystanders.set_vip(&players.get_player(match[1].str()));
}
