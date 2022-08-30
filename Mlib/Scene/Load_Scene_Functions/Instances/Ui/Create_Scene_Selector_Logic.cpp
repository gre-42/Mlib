#include "Create_Scene_Selector_Logic.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Render_Logics/Scene_Selector_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <list>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(TITLE);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(POSITION_X);
DECLARE_OPTION(POSITION_Y);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(SCENE_FILES);

LoadSceneUserFunction CreateSceneSelectorLogic::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*scene_selector"
        "\\s+id=([\\w+-.]+)"
        "\\s+title=([\\w+-. ]*)"
        "\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+scene_files=([\\s\\w+-.\\(\\)/:=%]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateSceneSelectorLogic(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateSceneSelectorLogic::CreateSceneSelectorLogic(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateSceneSelectorLogic::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::list<SceneEntry> scene_entries;
    for (const auto& e : find_all_name_values(match[SCENE_FILES].str(), "[\\w+-. \\(\\)/:]+", "[\\w+-. \\(\\)/:]+")) {
        scene_entries.push_back(SceneEntry{
            .name = e.first,
            .filename = args.fpath(e.second).path});
    }
    std::string id = match[ID].str();
    std::string title = match[TITLE].str();
    args.ui_focus.insert_submenu(id, title, 0);
    auto scene_selector_logic = std::make_shared<SceneSelectorLogic>(
        "",
        std::vector<SceneEntry>{scene_entries.begin(), scene_entries.end()},
        args.fpath(match[TTF_FILE].str()).path,       // ttf_filename
        FixedArray<float, 2>{                         // position
            safe_stof(match[POSITION_X].str()),
            safe_stof(match[POSITION_Y].str())},
        safe_stof(match[FONT_HEIGHT].str()),          // font_height_pixels
        safe_stof(match[LINE_DISTANCE].str()),        // line_distance_pixels
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_ids = { id } },
        args.next_scene_filename,
        button_press,
        args.ui_focus.selection_ids.at(id));
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = secondary_rendering_context.scene_node_resources,
        .rendering_resources = secondary_rendering_context.rendering_resources,
        .z_order = 1} };
    render_logics.append(nullptr, scene_selector_logic);
}
