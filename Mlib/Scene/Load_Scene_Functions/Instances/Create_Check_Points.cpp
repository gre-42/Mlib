#include "Create_Check_Points.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Layout/Layout_Constraints.hpp>
#include <Mlib/Layout/Widget.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Advance_Times/Check_Points.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Render_Logics/Render_Logics.hpp>
#include <Mlib/Scene/Render_Logics/Check_Points_Pacenotes.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(MOVING_NODE);
DECLARE_OPTION(RESOURCE);
DECLARE_OPTION(PLAYER);
DECLARE_OPTION(NBEACONS);
DECLARE_OPTION(NTH);
DECLARE_OPTION(NAHEAD);
DECLARE_OPTION(RADIUS);
DECLARE_OPTION(HEIGHT_CHANGED);
DECLARE_OPTION(TRACK_FILENAME);
DECLARE_OPTION(LAPS);
DECLARE_OPTION(PACENOTES_FILENAME);
DECLARE_OPTION(PACENOTES_METERS_AHEAD);
DECLARE_OPTION(PACENOTES_MINIMUM_COVERED_METERS);
DECLARE_OPTION(PACENOTES_MAXIMUM_NUMBER);
DECLARE_OPTION(PACENOTES_PICTURES_LEFT);
DECLARE_OPTION(PACENOTES_PICTURES_RIGHT);
DECLARE_OPTION(PACENOTES_TTF);
DECLARE_OPTION(PACENOTES_R);
DECLARE_OPTION(PACENOTES_G);
DECLARE_OPTION(PACENOTES_B);
DECLARE_OPTION(PACENOTES_WIDGET_DISTANCE);
DECLARE_OPTION(PACENOTES_TEXT_LEFT);
DECLARE_OPTION(PACENOTES_TEXT_RIGHT);
DECLARE_OPTION(PACENOTES_TEXT_BOTTOM);
DECLARE_OPTION(PACENOTES_TEXT_TOP);
DECLARE_OPTION(PACENOTES_PICTURE_LEFT);
DECLARE_OPTION(PACENOTES_PICTURE_RIGHT);
DECLARE_OPTION(PACENOTES_PICTURE_BOTTOM);
DECLARE_OPTION(PACENOTES_PICTURE_TOP);
DECLARE_OPTION(PACENOTES_FONT_HEIGHT);
DECLARE_OPTION(SELECTION_EMISSIVITY_R);
DECLARE_OPTION(SELECTION_EMISSIVITY_G);
DECLARE_OPTION(SELECTION_EMISSIVITY_B);
DECLARE_OPTION(DESELECTION_EMISSIVITY_R);
DECLARE_OPTION(DESELECTION_EMISSIVITY_G);
DECLARE_OPTION(DESELECTION_EMISSIVITY_B);
DECLARE_OPTION(ON_FINISH);

const std::string CreateCheckPoints::key = "check_points";

LoadSceneUserFunction CreateCheckPoints::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^moving_node=([\\w+-.]+),"
        "\\s+resource=([^,]+),"
        "\\s+player=([\\w+-.]+),"
        "\\s+nbeacons=(\\d+),"
        "\\s+nth=(\\d+),"
        "\\s+nahead=(\\d+),"
        "\\s+radius=([\\w+-.]+),"
        "\\s+height_changed=(0|1),"
        "\\s+track_filename=([^,]+),"
        "\\s+laps=(\\d+),"
        "(?:\\s+pacenotes_filename=([^,]*),"
        "\\s+pacenotes_meters_ahead=(\\d+),"
        "\\s+pacenotes_minimum_covered_meters=([^,]+),"
        "\\s+pacenotes_maximum_number=(\\d+),"
        "\\s+pacenotes_pictures_left=([^,]+),"
        "\\s+pacenotes_pictures_right=([^,]+),"
        "\\s+pacenotes_ttf=([^,]+),"
        "\\s+pacenotes_R=([^,]+),"
        "\\s+pacenotes_G=([^,]+),"
        "\\s+pacenotes_B=([^,]+),"
        "\\s+pacenotes_widget_distance=([^,]+),"
        "\\s+pacenotes_text_left=([^,]+),"
        "\\s+pacenotes_text_right=([^,]+),"
        "\\s+pacenotes_text_bottom=([^,]+),"
        "\\s+pacenotes_text_top=([^,]+),"
        "\\s+pacenotes_picture_left=([^,]+),"
        "\\s+pacenotes_picture_right=([^,]+),"
        "\\s+pacenotes_picture_bottom=([^,]+),"
        "\\s+pacenotes_picture_top=([^,]+),"
        "\\s+pacenotes_font_height=([^,]+),)?"
        "(?:\\s+selection_emissivity=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+),)?"
        "(?:\\s+deselection_emissivity=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+),)?"
        "\\s+on_finish=([\\w+-.:= ]*)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    CreateCheckPoints(args.renderable_scene()).execute(match, args);
};

CreateCheckPoints::CreateCheckPoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateCheckPoints::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& moving_node = scene.get_node(match[MOVING_NODE].str());
    std::string on_finish = match[ON_FINISH].str();
    size_t nlaps = safe_stoz(match[LAPS].str());
    auto check_points = std::make_unique<CheckPoints>(
        args.fpath(match[TRACK_FILENAME].str()).path,
        nlaps,
        args.scene_node_resources.get_geographic_mapping("world.inverse"),
        physics_engine.advance_times_,
        moving_node,
        moving_node.get_absolute_movable(),
        match[RESOURCE].str(),
        players.get_player(match[PLAYER].str()),
        safe_stoi(match[NBEACONS].str()),
        safe_stoi(match[NTH].str()),
        safe_stoi(match[NAHEAD].str()),
        safe_stof(match[RADIUS].str()),
        scene_node_resources,
        scene,
        delete_node_mutex,
        args.ui_focus.focuses,
        safe_stob(match[HEIGHT_CHANGED].str()),
        FixedArray<float, 3>{
            match[SELECTION_EMISSIVITY_R].matched ? safe_stof(match[SELECTION_EMISSIVITY_R].str()) : -1,
            match[SELECTION_EMISSIVITY_G].matched ? safe_stof(match[SELECTION_EMISSIVITY_G].str()) : -1,
            match[SELECTION_EMISSIVITY_B].matched ? safe_stof(match[SELECTION_EMISSIVITY_B].str()) : -1},
        FixedArray<float, 3>{
            match[DESELECTION_EMISSIVITY_R].matched ? safe_stof(match[DESELECTION_EMISSIVITY_R].str()) : -1,
            match[DESELECTION_EMISSIVITY_G].matched ? safe_stof(match[DESELECTION_EMISSIVITY_G].str()) : -1,
            match[DESELECTION_EMISSIVITY_B].matched ? safe_stof(match[DESELECTION_EMISSIVITY_B].str()) : -1},
        [on_finish, mle=args.macro_line_executor](){
            mle(on_finish, nullptr);
        });
    auto pacenotes_filename = match[PACENOTES_FILENAME].str();
    if (!pacenotes_filename.empty()) {
        auto text_widget = std::make_unique<Widget>(
            args.layout_constraints.get_pixels(match[PACENOTES_TEXT_LEFT].str()),
            args.layout_constraints.get_pixels(match[PACENOTES_TEXT_RIGHT].str()),
            args.layout_constraints.get_pixels(match[PACENOTES_TEXT_BOTTOM].str()),
            args.layout_constraints.get_pixels(match[PACENOTES_TEXT_TOP].str()));
        auto picture_widget = std::make_unique<Widget>(
            args.layout_constraints.get_pixels(match[PACENOTES_PICTURE_LEFT].str()),
            args.layout_constraints.get_pixels(match[PACENOTES_PICTURE_RIGHT].str()),
            args.layout_constraints.get_pixels(match[PACENOTES_PICTURE_BOTTOM].str()),
            args.layout_constraints.get_pixels(match[PACENOTES_PICTURE_TOP].str()));
        auto renderable_pace_notes = std::make_shared<CheckPointsPacenotes>(
            args.gallery,
            string_to_vector(match[PACENOTES_PICTURES_LEFT].str()),
            string_to_vector(match[PACENOTES_PICTURES_RIGHT].str()),
            args.layout_constraints.get_pixels(match[PACENOTES_WIDGET_DISTANCE].str()),
            std::move(text_widget),
            std::move(picture_widget),
            args.layout_constraints.get_pixels(match[PACENOTES_FONT_HEIGHT].str()),
            args.fpath(match[PACENOTES_TTF].str()).path,
            FixedArray<float, 3>{
                safe_stof(match[PACENOTES_R].str()),
                safe_stof(match[PACENOTES_G].str()),
                safe_stof(match[PACENOTES_B].str())},
            args.fpath(pacenotes_filename).path,
            *check_points,
            nlaps,
            safe_stod(match[PACENOTES_METERS_AHEAD].str()),
            safe_stod(match[PACENOTES_MINIMUM_COVERED_METERS].str()),
            safe_stoz(match[PACENOTES_MAXIMUM_NUMBER].str()),
            render_logics,
            physics_engine.advance_times_,
            moving_node);
        render_logics.append(nullptr, renderable_pace_notes);
        physics_engine.advance_times_.add_advance_time(*renderable_pace_notes);
    }
    physics_engine.advance_times_.add_advance_time(std::move(check_points));
}
