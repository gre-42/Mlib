#include "Create_Tab_Menu_Logic.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Render_Logics/List_View_Style.hpp>
#include <Mlib/Scene/Render_Logics/Tab_Menu_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Strings/To_Number.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(KEY);
DECLARE_OPTION(GAMEPAD_BUTTON);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS_SIGN);
DECLARE_OPTION(TAP_BUTTON);
DECLARE_OPTION(ID);
DECLARE_OPTION(SELECTION_MARKER);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(ICON_LEFT);
DECLARE_OPTION(ICON_RIGHT);
DECLARE_OPTION(ICON_BOTTOM);
DECLARE_OPTION(ICON_TOP);
DECLARE_OPTION(LEFT);
DECLARE_OPTION(RIGHT);
DECLARE_OPTION(BOTTOM);
DECLARE_OPTION(TOP);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(DEFAULT);
DECLARE_OPTION(RELOAD_TRANSIENT_OBJECTS);

LoadSceneUserFunction CreateTabMenuLogic::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*tab_menu"
        "\\s+key=([\\w+-.]+)"
        "(?:\\s+gamepad_button=([\\w+-.]+))?"
        "(?:\\s+joystick_digital_axis=([\\w+-.]+)"
        "\\s+joystick_digital_axis_sign=([\\w+-.]+))?"
        "(?:\\s+tap_button=([\\w+-.]+))?"
        "\\s+id=([\\w+-.]+)"
        "\\s+selection_marker=([\\w+-. \\(\\)/]+)"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        "\\s+icon_left=(\\w+)"
        "\\s+icon_right=(\\w+)"
        "\\s+icon_bottom=(\\w+)"
        "\\s+icon_top=(\\w+)"
        "\\s+left=(\\w+)"
        "\\s+right=(\\w+)"
        "\\s+bottom=(\\w+)"
        "\\s+top=(\\w+)"
        "\\s+font_height=(\\w+)"
        "\\s+line_distance=(\\w+)"
        "\\s+default=([\\d]+)"
        "\\s+reload_transient_objects=([\\w+-.:= ]*)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateTabMenuLogic(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateTabMenuLogic::CreateTabMenuLogic(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateTabMenuLogic::execute(const Mlib::re::smatch& match, const LoadSceneUserFunctionArgs& args)
{
    std::string id = match[ID].str();
    auto icon_widget = std::make_unique<Widget>(
        args.layout_constraints.get_pixels(match[ICON_LEFT].str()),
        args.layout_constraints.get_pixels(match[ICON_RIGHT].str()),
        args.layout_constraints.get_pixels(match[ICON_BOTTOM].str()),
        args.layout_constraints.get_pixels(match[ICON_TOP].str()));
    auto widget = std::make_unique<Widget>(
        args.layout_constraints.get_pixels(match[LEFT].str()),
        args.layout_constraints.get_pixels(match[RIGHT].str()),
        args.layout_constraints.get_pixels(match[BOTTOM].str()),
        args.layout_constraints.get_pixels(match[TOP].str()));
    size_t deflt = safe_stoz(match[DEFAULT].str());
    std::string reload_transient_objects = match[RELOAD_TRANSIENT_OBJECTS].str();
    // If the selection_ids array is not yet initialized, apply the default value.
    args.ui_focus.selection_ids.try_emplace(id, deflt);
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources,  // read by TabMenuLogic
        .rendering_resources = primary_rendering_context.rendering_resources,    // read by TabMenuLogic
        .z_order = 1} };                                                         // read by render_logics
    auto tab_menu_logic = std::make_shared<TabMenuLogic>(
        BaseKeyBinding{
            .key = match[KEY].str(),
            .gamepad_button = match[GAMEPAD_BUTTON].str(),
            .joystick_axis = match[JOYSTICK_DIGITAL_AXIS].str(),
            .joystick_axis_sign = match[JOYSTICK_DIGITAL_AXIS_SIGN].matched
                ? safe_stof(match[JOYSTICK_DIGITAL_AXIS_SIGN].str())
                : 0,
            .tap_button = match[TAP_BUTTON].str()},
        args.ui_focus.submenu_headers,
        args.gallery,
        ListViewStyle::ICON,
        match[SELECTION_MARKER].str(),
        args.fpath(match[TTF_FILE].str()).path,
        std::move(icon_widget),
        std::move(widget),
        args.layout_constraints.get_pixels(match[FONT_HEIGHT].str()),
        args.layout_constraints.get_pixels(match[LINE_DISTANCE].str()),
        args.external_substitutions,
        args.ui_focus,
        args.num_renderings,
        button_press,
        args.ui_focus.selection_ids.at(id),
        args.script_filename,
        args.next_scene_filename,
        [macro_line_executor = args.macro_line_executor, reload_transient_objects]() {
            if (!reload_transient_objects.empty()) {
                macro_line_executor(reload_transient_objects, nullptr);
                // This results in a deadlock because both "delete_node_mutex" and "delete_rigid_body_mutex" are acquired.
                // std::scoped_lock rb_lock{ delete_rigid_body_mutex };
                // macro_line_executor(reload_transient_objects, nullptr);
            }
        });
    render_logics.append(nullptr, tab_menu_logic);
}
