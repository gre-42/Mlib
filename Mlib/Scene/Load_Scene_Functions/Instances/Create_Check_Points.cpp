#include "Create_Check_Points.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Advance_Times/Check_Points.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

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
DECLARE_OPTION(DESELECTION_EMISSIVITY_R);
DECLARE_OPTION(DESELECTION_EMISSIVITY_G);
DECLARE_OPTION(DESELECTION_EMISSIVITY_B);
DECLARE_OPTION(ON_FINISH);

LoadSceneUserFunction CreateCheckPoints::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*check_points"
        "\\s+moving_node=([\\w+-.]+)"
        "\\s+resource=([\\w+-. \\(\\)/]+)"
        "\\s+player=([\\w+-.]+)"
        "\\s+nbeacons=(\\d+)"
        "\\s+nth=(\\d+)"
        "\\s+nahead=(\\d+)"
        "\\s+radius=([\\w+-.]+)"
        "\\s+height_changed=(0|1)"
        "\\s+track_filename=([\\w+-. \\(\\)/\\\\:]+)"
        "\\s+laps=(\\d+)"
        "(?:\\s+deselection_emissivity=([\\w+-.]+) ([\\w+-.]+) ([\\w+-.]+))?"
        "\\s+on_finish=([\\w+-.:= ]*)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateCheckPoints(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
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
    physics_engine.advance_times_.add_advance_time(std::make_shared<CheckPoints>(
        args.fpath(match[TRACK_FILENAME].str()).path,
        safe_stof(match[LAPS].str()),
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
            match[DESELECTION_EMISSIVITY_R].matched ? safe_stof(match[DESELECTION_EMISSIVITY_R].str()) : -1,
            match[DESELECTION_EMISSIVITY_G].matched ? safe_stof(match[DESELECTION_EMISSIVITY_G].str()) : -1,
            match[DESELECTION_EMISSIVITY_B].matched ? safe_stof(match[DESELECTION_EMISSIVITY_B].str()) : -1},
        [on_finish, mle=args.macro_line_executor, &rsc = args.rsc](){
            mle(on_finish, nullptr, rsc);
        }));

}
