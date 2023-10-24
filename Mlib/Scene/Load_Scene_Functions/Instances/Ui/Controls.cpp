#include "Controls.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Render/Render_Logics/Controls_Logic.hpp>
#include <Mlib/Render/Render_Logics/Delay_Load_Policy.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(icon);
DECLARE_ARGUMENT(gamepad_texture);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(delay_load_policy);
}

const std::string Controls::key = "controls";

LoadSceneJsonUserFunction Controls::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    Controls(args.renderable_scene()).execute(args);
};

Controls::Controls(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void Controls::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::string id = args.arguments.at<std::string>(KnownArgs::id);
    std::shared_ptr<ControlsLogic> controls_logic;
    args.ui_focus.insert_submenu(
        id,
        SubmenuHeader{
            .title = args.arguments.at<std::string>(KnownArgs::title),
            .icon = args.arguments.at<std::string>(KnownArgs::icon)},
        0);
    controls_logic = std::make_shared<ControlsLogic>(
        args.arguments.path(KnownArgs::gamepad_texture),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        delay_load_policy_from_string(args.arguments.at<std::string>(KnownArgs::delay_load_policy)),
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_ids = { id } });
    render_logics.append(nullptr, controls_logic, 1 /* z_order */);
}
