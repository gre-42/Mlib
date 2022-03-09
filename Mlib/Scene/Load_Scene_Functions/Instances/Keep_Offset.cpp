#include "Keep_Offset.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Keep_Offset_Movable.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

LoadSceneUserFunction KeepOffset::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*keep_offset"
        "\\s+follower=([\\w+-.]+)"
        "\\s+followed=([\\w+-.]+)"
        "\\s+offset=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        KeepOffset(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

KeepOffset::KeepOffset(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void KeepOffset::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& follower_node = scene.get_node(match[1].str());
    auto& followed_node = scene.get_node(match[2].str());
    auto follower = std::make_shared<KeepOffsetMovable>(
        physics_engine.advance_times_,
        scene,
        match[1].str(),
        &followed_node,
        followed_node.get_absolute_movable(),
        FixedArray<float, 3>{
            safe_stof(match[3].str()),
            safe_stof(match[4].str()),
            safe_stof(match[5].str())});
    linker.link_absolute_movable(follower_node, follower);
}
