#include "Set_Session_And_Reload_History.hpp"
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(SESSION);

LoadSceneUserFunction SetSessionAndReloadHistory::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_session_and_reload_history"
        "\\s+session=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetSessionAndReloadHistory(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetSessionAndReloadHistory::SetSessionAndReloadHistory(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetSessionAndReloadHistory::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    players.set_session_name_and_reload_history(match[SESSION].str());
}
