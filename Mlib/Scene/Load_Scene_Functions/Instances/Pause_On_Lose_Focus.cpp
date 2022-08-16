#include "Pause_On_Lose_Focus.hpp"
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Render_Logics/Pause_On_Lose_Focus_Logic.hpp>
#include <Mlib/Scene/Renderable_Scene.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(FOCUS_MASK);
DECLARE_OPTION(SUBMENUS);

LoadSceneUserFunction PauseOnLoseFocus::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*pause_on_lose_focus"
        "\\s+focus_mask=(menu|loading|countdown_any|scene)"
        "\\s+submenus=(.*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PauseOnLoseFocus(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PauseOnLoseFocus::PauseOnLoseFocus(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PauseOnLoseFocus::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto wit = args.renderable_scenes.find("primary_scene");
    if (wit == args.renderable_scenes.end()) {
        throw std::runtime_error("Could not find renderable scene with name \"primary_scene\"");
    }
    auto polf = std::make_shared<PauseOnLoseFocusLogic>(
        audio_paused,
        physics_set_fps,
        args.ui_focus,
        FocusFilter{
            .focus_mask = focus_from_string(match[FOCUS_MASK].str()),
            .submenu_ids = string_to_set(match[SUBMENUS].str())});
    wit->second.render_logics_.append(nullptr, polf);
}
