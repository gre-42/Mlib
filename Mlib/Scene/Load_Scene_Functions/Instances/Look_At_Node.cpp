#include "Look_At_Node.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Look_At_Movable.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

LoadSceneUserFunction LookAtNode::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*look_at_node"
        "\\s+follower=([\\w+-.]+)"
        "\\s+followed=([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        LookAtNode(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

LookAtNode::LookAtNode(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void LookAtNode::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& follower_node = scene.get_node(match[1].str());
    auto& followed_node = scene.get_node(match[2].str());
    auto follower = std::make_shared<LookAtMovable>(
        physics_engine.advance_times_,
        scene,
        match[1].str(),
        followed_node,
        followed_node.get_absolute_movable());
    linker.link_absolute_movable(follower_node, follower);

}
