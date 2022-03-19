#include "Follow_Node.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Follow_Movable.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/Scene_Config.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

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
    std::smatch match;
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
    auto& follower_node = scene.get_node(match[1].str());
    auto& followed_node = scene.get_node(match[2].str());
    auto distance = safe_stof(match[3].str());
    auto follower = std::make_shared<FollowMovable>(
        physics_engine.advance_times_,
        followed_node,
        followed_node.get_absolute_movable(),
        distance,                          // attachment_distance
        FixedArray<float, 3>{              // node_displacement
            safe_stof(match[4].str()),
            safe_stof(match[5].str()),
            safe_stof(match[6].str())},
        FixedArray<float, 3>{              // look_at_displacement
            safe_stof(match[7].str()),
            safe_stof(match[8].str()),
            safe_stof(match[9].str())},
        safe_stof(match[10].str()),        // snappiness
        safe_stof(match[11].str()),        // y_adaptivity
        safe_stof(match[12].str()),        // y_snappiness
        scene_config.physics_engine_config.dt / s);
    linker.link_absolute_movable(follower_node, follower);
    follower->initialize(follower_node);

}
