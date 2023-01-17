#include "Controls.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
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
DECLARE_OPTION(ICON);
DECLARE_OPTION(GAMEPAD_TEXTURE);
DECLARE_OPTION(LEFT);
DECLARE_OPTION(RIGHT);
DECLARE_OPTION(BOTTOM);
DECLARE_OPTION(TOP);

LoadSceneUserFunction Controls::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*controls"
        "\\s+id=([\\w+-.]+)"
        "\\s+title=([\\w+-. ]*)"
        "\\s+icon=(\\w+)"
        "\\s+gamepad_texture=(#?[\\w+-. \\(\\)/]+)"
        "\\s+left=(\\w+)"
        "\\s+right=(\\w+)"
        "\\s+bottom=(\\w+)"
        "\\s+top=(\\w+)$");
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
    std::string id = match[ID].str();
    std::shared_ptr<ControlsLogic> controls_logic;
    args.ui_focus.insert_submenu(
        id,
        SubmenuHeader{
            .title = match[TITLE].str(),
            .icon = match[ICON].str()},
        0);
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources, // read by ControlsLogic
        .rendering_resources = primary_rendering_context.rendering_resources,   // read by ControlsLogic
        .z_order = 1} };                                                        // read by render_logics
    controls_logic = std::make_shared<ControlsLogic>(
        args.fpath(match[GAMEPAD_TEXTURE].str()).path,
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(match[LEFT].str()),
            args.layout_constraints.get_pixels(match[RIGHT].str()),
            args.layout_constraints.get_pixels(match[BOTTOM].str()),
            args.layout_constraints.get_pixels(match[TOP].str())),
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_ids = { id } });
    render_logics.append(nullptr, controls_logic);
}
