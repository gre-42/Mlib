#include "Ui_Background.hpp"
#include <Mlib/FPath.hpp>
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
        "\\s+texture=([\\w+-. \\(\\)/]+)"
        "\\s+update=(\\w+)"
        "\\s+focus_mask=(\\w+)$");
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
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources,  // read by MainMenuBackgroundLogic/FillWithTextureLogic
        .rendering_resources = primary_rendering_context.rendering_resources,    // read by MainMenuBackgroundLogic/FillWithTextureLogic
        .z_order = 1} };                                                         // read by RenderLogics
    auto bg = std::make_shared<MainMenuBackgroundLogic>(
        args.fpath(match[1].str()).path,
        resource_update_cycle_from_string(match[2].str()),
        FocusFilter{ .focus_mask = focus_from_string(match[3].str()) });
    render_logics.append(nullptr, bg);
}
