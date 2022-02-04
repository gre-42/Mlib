#include "Set_Spawn_Points.hpp"
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(RESOURCE);

LoadSceneUserFunction SetSpawnPoints::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*set_spawn_points"
        "\\s+node=([\\w+-.]+)"
        "\\s+resource=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        SetSpawnPoints(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

SetSpawnPoints::SetSpawnPoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetSpawnPoints::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    SceneNode* node = scene.get_node(match[NODE].str());
    std::list<SpawnPoint> spawn_points = scene_node_resources.spawn_points(match[RESOURCE].str());
    game_logic.spawn.set_spawn_points(*node, spawn_points);
}
