#include "Controls.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Controls_Logic.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

LoadSceneUserFunction Controls::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*controls"
        "\\s+id=([\\w+-.]+)"
        "\\s+title=([\\w+-. ]*)"
        "\\s+gamepad_texture=(#?[\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+size=([\\w+-.]+)\\s+([\\w+-.]+)$");
    std::smatch match;
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
        args.fpath(match[3].str()).path,  // gamepad_texture
        FixedArray<float, 2>{             // position
            safe_stof(match[4].str()),
            safe_stof(match[5].str())},
        FixedArray<float, 2>{             // size
            safe_stof(match[6].str()),
            safe_stof(match[7].str())},
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_id = id });
    RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
    render_logics.append(nullptr, controls_logic);
}
