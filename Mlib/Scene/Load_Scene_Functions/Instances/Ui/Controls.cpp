#include "Controls.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Controls_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(TITLE);
DECLARE_OPTION(GAMEPAD_TEXTURE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);

LoadSceneUserFunction Controls::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*controls"
        "\\s+id=([\\w+-.]+)"
        "\\s+title=([\\w+-. ]*)"
        "\\s+gamepad_texture=(#?[\\w+-. \\(\\)/]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        Controls(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

Controls::Controls(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void Controls::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::string id = match[1].str();
    std::string title = match[2].str();
    std::shared_ptr<ControlsLogic> controls_logic;
    args.ui_focus.insert_submenu(id, title, 0);
    controls_logic = std::make_shared<ControlsLogic>(
        args.fpath(match[GAMEPAD_TEXTURE].str()).path,
        FixedArray<float, 2>{
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str())},
        FixedArray<float, 2>{
            safe_stof(match[SIZE_X].str()),
            safe_stof(match[SIZE_Y].str())},
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_ids = { id } });
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = secondary_rendering_context.scene_node_resources,
        .rendering_resources = secondary_rendering_context.rendering_resources,
        .z_order = 1} };
    render_logics.append(nullptr, controls_logic);
}
