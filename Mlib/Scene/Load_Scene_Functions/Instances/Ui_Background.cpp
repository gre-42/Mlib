#include "Ui_Background.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Main_Menu_Background_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>

using namespace Mlib;

LoadSceneUserFunction UiBackground::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*ui_background"
        "\\s+texture=([\\w-. \\(\\)/+-]+)"
        "\\s+update=(once|always)"
        "\\s+focus_mask=(menu|loading|countdown_any|scene)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        UiBackground(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

UiBackground::UiBackground(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void UiBackground::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    
    auto bg = std::make_shared<MainMenuBackgroundLogic>(
        args.fpath(match[1].str()).path,
        resource_update_cycle_from_string(match[2].str()),
        FocusFilter{ .focus_mask = focus_from_string(match[3].str()) });
    RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
    render_logics.append(nullptr, bg);

}
