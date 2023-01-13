#include "Create_Scene_Selector_Logic.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Screen_Units.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Macro_Manifest.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Render_Logics/Scene_Selector_Logic.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>

using namespace Mlib;
namespace fs = std::filesystem;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(TITLE);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(LEFT);
DECLARE_OPTION(RIGHT);
DECLARE_OPTION(BOTTOM);
DECLARE_OPTION(TOP);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(FONT_HEIGHT_UNITS);
DECLARE_OPTION(ON_CHANGE);
DECLARE_OPTION(SCENE_DIRECTORY);

LoadSceneUserFunction CreateSceneSelectorLogic::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*scene_selector"
        "\\s+id=([\\w+-.]+)"
        ",\\s+title=([\\w+-. ]*)"
        ",\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        ",\\s+left=(\\w+)"
        ",\\s+right=(\\w+)"
        ",\\s+bottom=(\\w+)"
        ",\\s+top=(\\w+)"
        ",\\s+font_height=([\\w+-.]+)"
        ",\\s+line_distance=([\\w+-.]+)"
        ",\\s+font_height_units=(\\w+)"
        "(?:,\\s+on_change=([^,]+))?"
        ",\\s+scene_directory=([^,]+)$");
    Mlib::re::smatch match;
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
    static DECLARE_REGEX(manifest_regex, "^manifest_.*\\.json$");
    std::list<SceneEntry> scene_entries;
    for (const auto& root : args.fpathes(match[SCENE_DIRECTORY].str())) {
        for (auto const& level_dir : list_dir(root)) {
            for (const auto& candidate_file : list_dir(level_dir)) {
                if (!Mlib::re::regex_match(candidate_file.path().filename().string(), manifest_regex)) {
                    continue;
                }
                auto path_string = candidate_file.path().string();
                MacroManifest mm{path_string};
                try {
                    std::string name = mm.variables.get_value("LEVEL_NAME");
                    if (mm.requires_.has_value()) {
                        for (const auto& r : mm.requires_.value()) {
                            if (!args.external_substitutions.get_bool(r)) {
                                goto skip;
                            }
                        }
                    }
                    scene_entries.push_back(SceneEntry{
                        .name = name,
                        .filename = path_string});
                    skip:;
                } catch (const std::runtime_error& e) {
                    throw std::runtime_error("Error processing manifest file \"" + path_string + "\": " + e.what());
                }
            }
        }
    }
    if (scene_entries.empty()) {
        THROW_OR_ABORT("Could not find a single scene file");
    }
    scene_entries.sort();
    std::string id = match[ID].str();
    args.ui_focus.insert_submenu(id, SubmenuHeader{.title=match[TITLE].str()}, 0);
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources,  // read by SceneSelectorLogic
        .rendering_resources = primary_rendering_context.rendering_resources,    // read by SceneSelectorLogic
        .z_order = 1} };                                                         // read by render_logics
    auto scene_selector_logic = std::make_shared<SceneSelectorLogic>(
        "",
        std::vector<SceneEntry>{scene_entries.begin(), scene_entries.end()},
        args.fpath(match[TTF_FILE].str()).path,       // ttf_filename
        std::make_unique<Widget>(
            args.layout_constraints.get(match[LEFT].str()),
            args.layout_constraints.get(match[RIGHT].str()),
            args.layout_constraints.get(match[BOTTOM].str()),
            args.layout_constraints.get(match[TOP].str())),
        safe_stof(match[FONT_HEIGHT].str()),          // font_height_pixels
        safe_stof(match[LINE_DISTANCE].str()),        // line_distance_pixels
        screen_units_from_string(match[FONT_HEIGHT_UNITS].str()),
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_ids = { id } },
        args.external_substitutions,
        args.next_scene_filename,
        button_press,
        args.ui_focus.selection_ids.at(id),
        [mle=args.macro_line_executor, on_change=match[ON_CHANGE].str(), &rsc=args.rsc]() {
            if (!on_change.empty()) {
                mle(on_change, nullptr, rsc);
            }
        });
    render_logics.append(nullptr, scene_selector_logic);
}
