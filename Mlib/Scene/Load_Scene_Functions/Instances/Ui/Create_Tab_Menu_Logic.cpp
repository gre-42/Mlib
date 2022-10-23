#include "Create_Tab_Menu_Logic.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Render_Logics/Tab_Menu_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(KEY);
DECLARE_OPTION(GAMEPAD_BUTTON);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS);
DECLARE_OPTION(JOYSTICK_DIGITAL_AXIS_SIGN);
DECLARE_OPTION(ID);
DECLARE_OPTION(TITLE);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(SIZE_X);
DECLARE_OPTION(SIZE_Y);
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
        "\\s+id=([\\w+-.]+)"
        "\\s+title=([\\w+-. ]*)"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "(?:\\s+size=([\\w+-.]+)\\s+([\\w+-.]+))?"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+default=([\\d]+)"
        "\\s+reload_transient_objects=([\\w+-.:= ]*)$");
    std::smatch match;
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

void CreateTabMenuLogic::execute(const std::smatch& match, const LoadSceneUserFunctionArgs& args)
{
    std::string id = match[ID].str();
    std::string title = match[TITLE].str();
    std::string ttf_filename = args.fpath(match[TTF_FILE].str()).path;
    FixedArray<float, 2> position{
        safe_stof(match[POSITION_X].str()),
        safe_stof(match[POSITION_Y].str())};
    FixedArray<float, 2> size{
        match[SIZE_X].matched ? safe_stof(match[SIZE_X].str()) : NAN,
        match[SIZE_Y].matched ? safe_stof(match[SIZE_Y].str()) : NAN};
    float font_height_pixels = safe_stof(match[FONT_HEIGHT].str());
    float line_distance_pixels = safe_stof(match[LINE_DISTANCE].str());
    size_t deflt = safe_stoz(match[DEFAULT].str());
    std::string reload_transient_objects = match[RELOAD_TRANSIENT_OBJECTS].str();
    // If the selection_ids array is not yet initialized, apply the default value.
    args.ui_focus.selection_ids.try_emplace(id, deflt);
    auto tab_menu_logic = std::make_shared<TabMenuLogic>(
        BaseKeyBinding{
            .key = match[KEY].str(),
            .gamepad_button = match[GAMEPAD_BUTTON].str(),
            .joystick_axis = match[JOYSTICK_DIGITAL_AXIS].str(),
            .joystick_axis_sign = match[JOYSTICK_DIGITAL_AXIS_SIGN].matched
                ? safe_stof(match[JOYSTICK_DIGITAL_AXIS_SIGN].str())
                : 0},
        title,
        args.ui_focus.submenu_titles,
        ttf_filename,
        position,
        size,
        font_height_pixels,
        line_distance_pixels,
        args.ui_focus,
        args.num_renderings,
        button_press,
        args.ui_focus.selection_ids.at(id),
        args.script_filename,
        args.next_scene_filename,
        [macro_line_executor = args.macro_line_executor, reload_transient_objects, &rsc = args.rsc]() {
            if (!reload_transient_objects.empty()) {
                macro_line_executor(reload_transient_objects, nullptr, rsc);
                // This results in a deadlock because both "delete_node_mutex" and "delete_rigid_body_mutex" are acquired.
                // std::lock_guard rb_lock{ delete_rigid_body_mutex };
                // macro_line_executor(reload_transient_objects, nullptr, rsc);
            }
        });
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = secondary_rendering_context.scene_node_resources,
        .rendering_resources = secondary_rendering_context.rendering_resources,
        .z_order = 1} };
    render_logics.append(nullptr, tab_menu_logic);
}
