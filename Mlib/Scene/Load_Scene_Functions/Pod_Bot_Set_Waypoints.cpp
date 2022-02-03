#include "Pod_Bot_Set_Waypoints.hpp"
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Players/Mlib_Pod_Bot/Set_Pod_Bot_Way_Points.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(RESOURCE);

LoadSceneUserFunction PodBotSetWaypoints::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_pod_bot_way_points"
        "\\s+node=([\\w+-.]+)"
        "\\s+resource=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PodBotSetWaypoints(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PodBotSetWaypoints::PodBotSetWaypoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PodBotSetWaypoints::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    SceneNode* node = scene.get_node(match[NODE].str());
    std::map<WayPointLocation, PointsAndAdjacency<float, 3>> way_points = scene_node_resources.way_points(match[RESOURCE].str());
    set_pod_bot_way_points(*node, way_points);
}
