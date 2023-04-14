#include "Create_Scene_Selector_Logic.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Macro_Manifest.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Load_Scene_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Scene_Selector_Logic.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>

using namespace Mlib;
namespace fs = std::filesystem;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(ID);
DECLARE_OPTION(TITLE);
DECLARE_OPTION(ICON);
DECLARE_OPTION(TTF_FILE);
DECLARE_OPTION(LEFT);
DECLARE_OPTION(RIGHT);
DECLARE_OPTION(BOTTOM);
DECLARE_OPTION(TOP);
DECLARE_OPTION(FONT_HEIGHT);
DECLARE_OPTION(LINE_DISTANCE);
DECLARE_OPTION(ON_CHANGE);
DECLARE_OPTION(ASSETS);
DECLARE_OPTION(ASSET_PREFIX);

const std::string CreateSceneSelectorLogic::key = "scene_selector";

LoadSceneUserFunction CreateSceneSelectorLogic::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^id=([\\w+-.]+)"
        ",\\s+title=([\\w+-. ]*)"
        ",\\s+icon=(\\w+)"
        ",\\s+ttf_file=([\\w+-. \\(\\)/]+)"
        ",\\s+left=(\\w+)"
        ",\\s+right=(\\w+)"
        ",\\s+bottom=(\\w+)"
        ",\\s+top=(\\w+)"
        ",\\s+font_height=(\\w+)"
        ",\\s+line_distance=(\\w+)"
        "(?:,\\s+on_change=([^,]+))?"
        ",\\s+assets=([^,]+)"
        "(?:,\\s+asset_prefix=([^,]+))?$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreateSceneSelectorLogic(args.renderable_scene()).execute(match, args);
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
    for (const auto& mm : args.asset_references.get_macro_manifests(match[ASSETS].str())) {
        try {
            scene_entries.push_back(SceneEntry{
                .name = mm.manifest.name,
                .filename = mm.filename,
                .requires_ = mm.manifest.text_requires_});
            scene_entries.back().variables.merge(mm.manifest.text_variables, match[ASSET_PREFIX].str());
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Error processing manifest file \"" + mm.filename + "\": " + e.what());
        }
    }
    if (scene_entries.empty()) {
        THROW_OR_ABORT("Could not find a single scene file");
    }
    scene_entries.sort();
    std::string id = match[ID].str();
    args.ui_focus.insert_submenu(
        id,
        SubmenuHeader{
            .title=match[TITLE].str(),
            .icon=match[ICON].str()},
        0);
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources,  // read by SceneSelectorLogic
        .rendering_resources = primary_rendering_context.rendering_resources,    // read by SceneSelectorLogic
        .z_order = 1} };                                                         // read by render_logics
    auto scene_selector_logic = std::make_shared<SceneSelectorLogic>(
        "",
        std::vector<SceneEntry>{scene_entries.begin(), scene_entries.end()},
        args.fpath(match[TTF_FILE].str()).path,
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(match[LEFT].str()),
            args.layout_constraints.get_pixels(match[RIGHT].str()),
            args.layout_constraints.get_pixels(match[BOTTOM].str()),
            args.layout_constraints.get_pixels(match[TOP].str())),
        args.layout_constraints.get_pixels(match[FONT_HEIGHT].str()),
        args.layout_constraints.get_pixels(match[LINE_DISTANCE].str()),
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_ids = { id } },
        args.external_substitutions,
        args.next_scene_filename,
        button_press,
        args.ui_focus.selection_ids.at(id),
        [mle=args.macro_line_executor, on_change=match[ON_CHANGE].str()]()
        {
            if (!on_change.empty()) {
                mle(on_change, nullptr);
            }
        });
    render_logics.append(nullptr, scene_selector_logic);
}
