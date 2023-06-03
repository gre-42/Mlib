#include "Create_Scene_Selector_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Macro_Manifest.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Scene_Selector_Logic.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>

using namespace Mlib;
namespace fs = std::filesystem;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(title);
DECLARE_ARGUMENT(icon);
DECLARE_ARGUMENT(ttf_file);
DECLARE_ARGUMENT(left);
DECLARE_ARGUMENT(right);
DECLARE_ARGUMENT(bottom);
DECLARE_ARGUMENT(top);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(on_change);
DECLARE_ARGUMENT(assets);
DECLARE_ARGUMENT(asset_prefix);
}

const std::string CreateSceneSelectorLogic::key = "scene_selector";

LoadSceneJsonUserFunction CreateSceneSelectorLogic::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateSceneSelectorLogic(args.renderable_scene()).execute(args);
};

CreateSceneSelectorLogic::CreateSceneSelectorLogic(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateSceneSelectorLogic::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    std::list<SceneEntry> scene_entries;
    for (const auto& mm : args.asset_references.get_macro_manifests(args.arguments.at<std::string>(KnownArgs::assets))) {
        try {
            scene_entries.push_back(SceneEntry{
                .name = mm.manifest.name,
                .filename = mm.filename,
                .requires_ = mm.manifest.required});
            scene_entries.back().globals.merge(mm.manifest.globals, args.arguments.at<std::string>(KnownArgs::asset_prefix, ""));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Error processing manifest file \"" + mm.filename + "\": " + e.what());
        }
    }
    if (scene_entries.empty()) {
        THROW_OR_ABORT("Could not find a single scene file");
    }
    scene_entries.sort();
    std::string id = args.arguments.at<std::string>(KnownArgs::id);
    args.ui_focus.insert_submenu(
        id,
        SubmenuHeader{
            .title=args.arguments.at<std::string>(KnownArgs::title),
            .icon=args.arguments.at<std::string>(KnownArgs::icon)},
        0);
    RenderingContextGuard rcg{ RenderingContext{
        .scene_node_resources = primary_rendering_context.scene_node_resources,  // read by SceneSelectorLogic
        .particle_resources = primary_rendering_context.particle_resources,      // read by SceneSelectorLogic
        .rendering_resources = primary_rendering_context.rendering_resources,    // read by SceneSelectorLogic
        .z_order = 1} };                                                         // read by render_logics
    auto scene_selector_logic = std::make_shared<SceneSelectorLogic>(
        "",
        std::vector<SceneEntry>{scene_entries.begin(), scene_entries.end()},
        args.arguments.path(KnownArgs::ttf_file),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        FocusFilter{
            .focus_mask = Focus::MENU,
            .submenu_ids = { id } },
        args.external_json_macro_arguments,
        args.next_scene_filename,
        button_press,
        args.ui_focus.selection_ids.at(id),
        [mle=args.macro_line_executor, on_change=args.arguments.at(KnownArgs::on_change)]()
        {
            if (!on_change.empty()) {
                mle(JsonView{on_change}, nullptr, nullptr);
            }
        });
    render_logics.append(nullptr, scene_selector_logic);
}
