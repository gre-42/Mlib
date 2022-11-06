#include "Follow_Node.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Follow_Movable.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(FOLLOWER);
DECLARE_OPTION(FOLLOWED);
DECLARE_OPTION(DISTANCE);
DECLARE_OPTION(NODE_DISPLACEMENT_X);
DECLARE_OPTION(NODE_DISPLACEMENT_Y);
DECLARE_OPTION(NODE_DISPLACEMENT_Z);
DECLARE_OPTION(LOOK_AT_DISPLACEMENT_X);
DECLARE_OPTION(LOOK_AT_DISPLACEMENT_Y);
DECLARE_OPTION(LOOK_AT_DISPLACEMENT_Z);
DECLARE_OPTION(SNAPPINESS);
DECLARE_OPTION(Y_ADAPTIVITY);
DECLARE_OPTION(Y_SNAPPINESS);

LoadSceneUserFunction FollowNode::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*follow_node"
        "\\s+follower=([\\w+-.]+)"
        "\\s+followed=([\\w+-.]+)"
        "\\s+distance=([\\w+-.]+)"
        "\\s+node_displacement=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+look_at_displacement=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)"
        "\\s+snappiness=([\\w+-.]+)"
        "\\s+y_adaptivity=([\\w+-.]+)"
        "\\s+y_snappiness=([\\w+-.]+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        FollowNode(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

FollowNode::FollowNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void FollowNode::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& follower_node = scene.get_node(match[FOLLOWER].str());
    auto& followed_node = scene.get_node(match[FOLLOWED].str());
    auto distance = safe_stof(match[DISTANCE].str());
    auto follower = std::make_shared<FollowMovable>(
        physics_engine.advance_times_,
        followed_node,
        followed_node.get_absolute_movable(),
        distance,
        FixedArray<float, 3>{
            safe_stof(match[NODE_DISPLACEMENT_X].str()),
            safe_stof(match[NODE_DISPLACEMENT_Y].str()),
            safe_stof(match[NODE_DISPLACEMENT_Z].str())},
        FixedArray<float, 3>{
            safe_stof(match[LOOK_AT_DISPLACEMENT_X].str()),
            safe_stof(match[LOOK_AT_DISPLACEMENT_Y].str()),
            safe_stof(match[LOOK_AT_DISPLACEMENT_Z].str())},
        safe_stof(match[SNAPPINESS].str()),
        safe_stof(match[Y_ADAPTIVITY].str()),
        safe_stof(match[Y_SNAPPINESS].str()),
        scene_config.physics_engine_config.dt);
    linker.link_absolute_movable_and_additional_node(
        follower_node, followed_node, follower);
    follower->initialize(follower_node);
}
