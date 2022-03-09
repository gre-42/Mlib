#include "Player_Set_Node.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(PLAYER_NAME);
DECLARE_OPTION(NODE);

LoadSceneUserFunction PlayerSetNode::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*player_set_node"
        "\\s+player_name=([\\w+-.]+)"
        "\\s+node=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PlayerSetNode(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PlayerSetNode::PlayerSetNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlayerSetNode::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto& node = scene.get_node(match[NODE].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(node.get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Follower movable is not a rigid body");
    }
    players.get_player(match[PLAYER_NAME].str()).set_rigid_body(
        PlayerVehicle{
            .scene_node_name = match[NODE].str(),
            .scene_node = &node,
            .rb = rb});
}
