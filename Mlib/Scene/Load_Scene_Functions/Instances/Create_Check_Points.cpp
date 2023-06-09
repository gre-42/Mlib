#include "Create_Check_Points.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Physics/Advance_Times/Check_Points.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Check_Points_Pacenotes.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
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
DECLARE_ARGUMENT(nth);
DECLARE_ARGUMENT(nahead);
DECLARE_ARGUMENT(radius);
DECLARE_ARGUMENT(height_changed);
DECLARE_ARGUMENT(track_filename);
DECLARE_ARGUMENT(laps);
DECLARE_ARGUMENT(pacenotes_filename);
DECLARE_ARGUMENT(pacenotes_meters_ahead);
DECLARE_ARGUMENT(pacenotes_minimum_covered_meters);
DECLARE_ARGUMENT(pacenotes_maximum_number);
DECLARE_ARGUMENT(pacenotes_pictures_left);
DECLARE_ARGUMENT(pacenotes_pictures_right);
DECLARE_ARGUMENT(pacenotes_ttf);
DECLARE_ARGUMENT(pacenotes_color);
DECLARE_ARGUMENT(pacenotes_widget_distance);
DECLARE_ARGUMENT(pacenotes_text_left);
DECLARE_ARGUMENT(pacenotes_text_right);
DECLARE_ARGUMENT(pacenotes_text_bottom);
DECLARE_ARGUMENT(pacenotes_text_top);
DECLARE_ARGUMENT(pacenotes_picture_left);
DECLARE_ARGUMENT(pacenotes_picture_right);
DECLARE_ARGUMENT(pacenotes_picture_bottom);
DECLARE_ARGUMENT(pacenotes_picture_top);
DECLARE_ARGUMENT(pacenotes_font_height);
DECLARE_ARGUMENT(selection_emissivity);
DECLARE_ARGUMENT(deselection_emissivity);
DECLARE_ARGUMENT(on_finish);
}

const std::string CreateCheckPoints::key = "check_points";

LoadSceneJsonUserFunction CreateCheckPoints::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateCheckPoints(args.renderable_scene()).execute(args);
};

CreateCheckPoints::CreateCheckPoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCheckPoints::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto moving_asset_id = args.arguments.at<std::string>(KnownArgs::moving_asset_id);
    const auto& vars = args
        .asset_references
        .get_replacement_parameters("vehicles")
        .at(moving_asset_id);
    auto prefixes = vars.globals.at<std::vector<std::string>>("NODE_PREFIXES");
    auto suffix = args.arguments.at<std::string>(KnownArgs::moving_suffix);

    if (prefixes.empty()) {
        THROW_OR_ABORT("Prefixes cannot be empty");
    }

    std::vector<SceneNode*> moving_nodes;
    moving_nodes.reserve(prefixes.size());
    for (const auto& p : prefixes) {
        moving_nodes.push_back(&scene.get_node(p + suffix));
    }
    auto on_finish = args.arguments.at(KnownArgs::on_finish);
    size_t nlaps = args.arguments.at<size_t>(KnownArgs::laps);
    auto check_points = std::make_unique<CheckPoints>(
        args.arguments.path(KnownArgs::track_filename),
        nlaps,
        scene_node_resources.get_geographic_mapping("world.inverse"),
        physics_engine.advance_times_,
        moving_asset_id,
        moving_nodes,
        args.arguments.at<std::string>(KnownArgs::resource),
        players.get_player(args.arguments.at<std::string>(KnownArgs::player)),
        args.arguments.at<size_t>(KnownArgs::nbeacons),
        args.arguments.at<size_t>(KnownArgs::nth),
        args.arguments.at<size_t>(KnownArgs::nahead),
        args.arguments.at<float>(KnownArgs::radius),
        scene_node_resources,
        scene,
        delete_node_mutex,
        args.ui_focus.focuses,
        args.arguments.at<bool>(KnownArgs::height_changed),
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::selection_emissivity, fixed_full<float, 3>(-1.f)),
        args.arguments.at<FixedArray<float, 3>>(KnownArgs::deselection_emissivity, fixed_full<float, 3>(-1.f)),
        [on_finish, mle=args.macro_line_executor](){
            mle(JsonView{on_finish}, nullptr, nullptr);
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
        auto renderable_pace_notes = std::make_shared<CheckPointsPacenotes>(
            args.gallery,
            args.arguments.at<std::vector<std::string>>(KnownArgs::pacenotes_pictures_left),
            args.arguments.at<std::vector<std::string>>(KnownArgs::pacenotes_pictures_right),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_widget_distance)),
            std::move(text_widget),
            std::move(picture_widget),
            args.layout_constraints.get_pixels(args.arguments.at<std::string>(KnownArgs::pacenotes_font_height)),
            args.arguments.path(KnownArgs::pacenotes_ttf),
            args.arguments.at<FixedArray<float, 3>>(KnownArgs::pacenotes_color),
            args.arguments.path(KnownArgs::pacenotes_filename),
            *check_points,
            nlaps,
            args.arguments.at<double>(KnownArgs::pacenotes_meters_ahead),
            args.arguments.at<double>(KnownArgs::pacenotes_minimum_covered_meters),
            args.arguments.at<size_t>(KnownArgs::pacenotes_maximum_number),
            render_logics,
            physics_engine.advance_times_,
            **moving_nodes.begin());
        render_logics.append(nullptr, renderable_pace_notes);
        physics_engine.advance_times_.add_advance_time(*renderable_pace_notes);
    }
    physics_engine.advance_times_.add_advance_time(std::move(check_points));
}
