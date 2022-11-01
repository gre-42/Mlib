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

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(Z_ORDER);
DECLARE_OPTION(TEXTURE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);
DECLARE_OPTION(UPDATE);
DECLARE_OPTION(FOCUS_MASK);

LoadSceneUserFunction UiBackground::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*ui_background"
        "\\s+z_order=(\\d+)"
        "\\s+texture=([\\w+-. \\(\\)/]+)"
        "(?:\\s+position=([\\w+-.]+)\\s+([\\w+-.]+))?"
        "(?:\\s+size=([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+update=(\\w+)"
        "\\s+focus_mask=(\\w+)$");
    Mlib::re::smatch match;
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
        .z_order = safe_stoi(match[Z_ORDER].str())} };                           // read by RenderLogics
    auto bg = std::make_shared<MainMenuBackgroundLogic>(
        args.fpath(match[TEXTURE].str()).path,
        FixedArray<float, 2>{
            match[POSITION_X].matched ? safe_stof(match[POSITION_X].str()) : 0,
            match[POSITION_Y].matched ? safe_stof(match[POSITION_Y].str()) : 0},
        FixedArray<float, 2>{
            match[SIZE_X].matched ? safe_stof(match[SIZE_X].str()) : NAN,
            match[SIZE_Y].matched ? safe_stof(match[SIZE_Y].str()) : NAN},
        resource_update_cycle_from_string(match[UPDATE].str()),
        FocusFilter{ .focus_mask = focus_from_string(match[FOCUS_MASK].str()) });
    render_logics.append(nullptr, bg);
}
