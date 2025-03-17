#include "Create_Check_Points.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Expression_Watcher.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Focus_Filter.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Check_Points.hpp>
#include <Mlib/Physics/Misc/Track_Element_File.hpp>
#include <Mlib/Physics/Misc/Track_Element_Vector.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Check_Points_Pacenotes.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(moving_asset_id);
DECLARE_ARGUMENT(moving_suffix);
DECLARE_ARGUMENT(resource);
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(nbeacons);
DECLARE_ARGUMENT(distance);
DECLARE_ARGUMENT(nahead);
DECLARE_ARGUMENT(radius);
DECLARE_ARGUMENT(height_changed);
DECLARE_ARGUMENT(track_filename);
DECLARE_ARGUMENT(track);
DECLARE_ARGUMENT(circular);
DECLARE_ARGUMENT(laps);
DECLARE_ARGUMENT(pacenotes_filename);
DECLARE_ARGUMENT(pacenotes_meters_ahead);
DECLARE_ARGUMENT(pacenotes_minimum_covered_meters);
DECLARE_ARGUMENT(pacenotes_maximum_number);
DECLARE_ARGUMENT(pacenotes_pictures_left);
DECLARE_ARGUMENT(pacenotes_pictures_right);
DECLARE_ARGUMENT(pacenotes_charset);
DECLARE_ARGUMENT(pacenotes_ttf);
DECLARE_ARGUMENT(pacenotes_font_color);
DECLARE_ARGUMENT(pacenotes_font_height);
DECLARE_ARGUMENT(pacenotes_widget_distance);
DECLARE_ARGUMENT(pacenotes_text_left);
DECLARE_ARGUMENT(pacenotes_text_right);
DECLARE_ARGUMENT(pacenotes_text_bottom);
DECLARE_ARGUMENT(pacenotes_text_top);
DECLARE_ARGUMENT(pacenotes_picture_left);
DECLARE_ARGUMENT(pacenotes_picture_right);
DECLARE_ARGUMENT(pacenotes_picture_bottom);
DECLARE_ARGUMENT(pacenotes_picture_top);
DECLARE_ARGUMENT(selection_emissivity);
DECLARE_ARGUMENT(deselection_emissivity);
DECLARE_ARGUMENT(respawn_config);
DECLARE_ARGUMENT(on_finish);
DECLARE_ARGUMENT(focus_mask);
DECLARE_ARGUMENT(submenus);
}

CreateCheckPoints::CreateCheckPoints(RenderableScene& renderable_scene) 
    : LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCheckPoints::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto moving_asset_id = args.arguments.at<std::string>(KnownArgs::moving_asset_id);
    auto on_finish = args.arguments.at(KnownArgs::on_finish);
    size_t nframes = args.arguments.at<bool>(KnownArgs::circular) ? 1 : 0;
    size_t nlaps = args.arguments.at<size_t>(KnownArgs::laps);
    if (args.arguments.contains_non_null(KnownArgs::track_filename) ==
        args.arguments.contains_non_null(KnownArgs::track))
    {
        THROW_OR_ABORT("Require exactly one of \"track\" and \"track_filename\"");
    }
    std::unique_ptr<ITrackElementSequence> sequence;
    if (args.arguments.contains_non_null(KnownArgs::track_filename)) {
        auto filename = args.arguments.path(KnownArgs::track_filename);
        sequence = std::make_unique<TrackElementFile>(create_ifstream(filename), filename);
    } else {
        sequence = std::make_unique<TrackElementVector>(args.arguments.at<std::vector<std::vector<double>>>(KnownArgs::track));
    }
    auto& check_points = global_object_pool.create<CheckPoints>(
        CURRENT_SOURCE_LOCATION,
        std::move(sequence),
        nframes,
        nlaps,
        scene_node_resources.get_geographic_mapping("world.inverse"),
        moving_asset_id,
        args.arguments.at<std::string>(KnownArgs::resource),
        players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION),
        args.arguments.at<size_t>(KnownArgs::nbeacons),
        args.arguments.at<float>(KnownArgs::distance) * meters,
        args.arguments.at<size_t>(KnownArgs::nahead),
        args.arguments.at<float>(KnownArgs::radius) * meters,
        &rendering_resources,
        scene_node_resources,
        scene,
        delete_node_mutex,
        args.ui_focus.focuses,
        args.arguments.at<bool>(KnownArgs::height_changed),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::selection_emissivity, fixed_full<float, 3>(-1.f)),
        args.arguments.at<UFixedArray<float, 3>>(KnownArgs::deselection_emissivity, fixed_full<float, 3>(-1.f)),
        args.arguments.at<RespawnConfig>(KnownArgs::respawn_config),
        [on_finish, mle = args.macro_line_executor]() {
            mle(on_finish, nullptr, nullptr);
        });
    auto pacenotes_filename = args.arguments.path(KnownArgs::pacenotes_filename, "");
    if (!pacenotes_filename.empty()) {
        auto text_widget = std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_text_left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_text_right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_text_bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_text_top)));
        auto picture_widget = std::make_unique<Widget>(
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_picture_left)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_picture_right)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_picture_bottom)),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_picture_top)));
        auto& renderable_pace_notes = global_object_pool.create<CheckPointsPacenotes>(
            CURRENT_SOURCE_LOCATION,
            args.gallery,
            args.arguments.at<std::vector<std::string>>(KnownArgs::pacenotes_pictures_left),
            args.arguments.at<std::vector<std::string>>(KnownArgs::pacenotes_pictures_right),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_widget_distance)),
            std::move(text_widget),
            std::move(picture_widget),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_font_height)),
            std::make_unique<ExpressionWatcher>(args.macro_line_executor),
            args.arguments.at<std::string>(KnownArgs::pacenotes_charset),
            args.arguments.path(KnownArgs::pacenotes_ttf),
            args.arguments.at<UFixedArray<float, 3>>(KnownArgs::pacenotes_font_color),
            args.arguments.path(KnownArgs::pacenotes_filename),
            DanglingBaseClassRef<const CheckPoints>{ check_points, CURRENT_SOURCE_LOCATION },
            nlaps,
            args.arguments.at<double>(KnownArgs::pacenotes_meters_ahead),
            args.arguments.at<double>(KnownArgs::pacenotes_minimum_covered_meters),
            args.arguments.at<size_t>(KnownArgs::pacenotes_maximum_number),
            FocusFilter{
                .focus_mask = focus_from_string(args.arguments.at<std::string>(KnownArgs::focus_mask)),
                .submenu_ids = string_to_set(args.arguments.at<std::string>(KnownArgs::submenus, {}))});
        render_logics.append({ renderable_pace_notes, CURRENT_SOURCE_LOCATION }, 0 /* z_order */, CURRENT_SOURCE_LOCATION);
        physics_engine.advance_times_.add_advance_time({ renderable_pace_notes, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
        renderable_pace_notes.preload();
    }
    physics_engine.advance_times_.add_advance_time({ check_points, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}

namespace {

static struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "check_points",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateCheckPoints(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
