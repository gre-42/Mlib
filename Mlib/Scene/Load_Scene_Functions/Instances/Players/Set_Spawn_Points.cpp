#include "Set_Spawn_Points.hpp"
#include <Mlib/Players/Advance_Times/Game_Logic.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(RESOURCE);

const std::string SetSpawnPoints::key = "set_spawn_points";

LoadSceneUserFunction SetSpawnPoints::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^node=([\\w+-.]+)"
        "\\s+resource=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    SetSpawnPoints(args.renderable_scene()).execute(match, args);
};

SetSpawnPoints::SetSpawnPoints(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetSpawnPoints::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    std::list<SpawnPoint> spawn_points = scene_node_resources.spawn_points(match[RESOURCE].str());
    game_logic.spawn.set_spawn_points(node, spawn_points);
}
