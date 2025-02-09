#include "Create_Scene_Selector_Logic.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
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
DECLARE_ARGUMENT(font_color);
DECLARE_ARGUMENT(font_height);
DECLARE_ARGUMENT(line_distance);
DECLARE_ARGUMENT(on_change);
DECLARE_ARGUMENT(assets);
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
    for (const auto& [_, rpe] : args.asset_references[args.arguments.at<std::string>(KnownArgs::assets)]) {
        scene_entries.emplace_back(rpe);
    }
    if (scene_entries.empty()) {
        THROW_OR_ABORT("Could not find a single scene file");
    }
    scene_entries.sort();
    auto id = args.arguments.at<std::string>(KnownArgs::id);
    auto focus_filter = FocusFilter{
        .focus_mask = Focus::NEW_GAME_MENU,
        .submenu_ids = { id } };
    args.ui_focus.insert_submenu(
        id,
        SubmenuHeader{
            .title=args.arguments.at<std::string>(KnownArgs::title),
            .icon=args.arguments.at<std::string>(KnownArgs::icon)},
        focus_filter.focus_mask,
        0);
    auto& scene_selector_logic = object_pool.create<SceneSelectorLogic>(
        CURRENT_SOURCE_LOCATION,
        "id = " + id,
        std::vector<SceneEntry>{scene_entries.begin(), scene_entries.end()},
        args.arguments.path(KnownArgs::ttf_file),
        std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::top))),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::font_color),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::font_height)),
        args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::line_distance)),
        focus_filter,
        args.macro_line_executor,
        args.next_scene_filename,
        args.button_states,
        args.ui_focus.all_selection_ids.at(id),
        [mle=args.macro_line_executor, on_change=args.arguments.try_at(KnownArgs::on_change)]()
        {
            if (on_change.has_value()) {
                mle(*on_change, nullptr, nullptr);
            }
        });
    render_logics.append(
        { scene_selector_logic, CURRENT_SOURCE_LOCATION },
        1 /* z_order */,
        CURRENT_SOURCE_LOCATION);
}
