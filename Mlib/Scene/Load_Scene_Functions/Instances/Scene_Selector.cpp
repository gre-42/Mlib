#include "Scene_Selector.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Render_Logics/Scene_Selector_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <list>

using namespace Mlib;

LoadSceneUserFunction SceneSelector::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*scene_selector"
        "\\s+id=([\\w+-.]+)"
        "\\s+title=([\\w+-. ]*)"
        "\\s+ttf_file=([\\w-. \\(\\)/+-]+)"
        "\\s+position=([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+font_height=([\\w+-.]+)"
        "\\s+line_distance=([\\w+-.]+)"
        "\\s+scene_files=([\\s\\w+-.\\(\\)/:=%]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SceneSelector(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SceneSelector::SceneSelector(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SceneSelector::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    std::list<SceneEntry> scene_entries;
    for (const auto& e : find_all_name_values(match[8].str(), "[\\w+-. \\(\\)/:]+", "[\\w+-. \\(\\)/:]+")) {
        scene_entries.push_back(SceneEntry{
            .name = e.first,
            .filename = args.fpath(e.second).path});
    }
    std::string id = match[1].str();
    std::string title = match[2].str();
    args.ui_focus.insert_submenu(id, title, 0);
    auto scene_selector_logic = std::make_shared<SceneSelectorLogic>(
        "",
        std::vector<SceneEntry>{scene_entries.begin(), scene_entries.end()},
        args.fpath(match[3].str()).path,       // ttf_filename
        FixedArray<float, 2>{             // position
            safe_stof(match[4].str()),
            safe_stof(match[5].str())},
        safe_stof(match[6].str()),        // font_height_pixels
        safe_stof(match[7].str()),        // line_distance_pixels
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_id = id },
        args.next_scene_filename,
        button_press,
        args.ui_focus.selection_ids.at(id));
    RenderingContextGuard rcg{ RenderingContext {.rendering_resources = secondary_rendering_context.rendering_resources, .z_order = 1} };
    render_logics.append(nullptr, scene_selector_logic);
}
