#include "Set_Vip.hpp"
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>

using namespace Mlib;

const std::string SetVip::key = "set_vip";

LoadSceneUserFunction SetVip::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^player=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    SetVip(args.renderable_scene()).execute(match, args);
};

SetVip::SetVip(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetVip::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    game_logic.bystanders.set_vip(&players.get_player(match[1].str()));
}
