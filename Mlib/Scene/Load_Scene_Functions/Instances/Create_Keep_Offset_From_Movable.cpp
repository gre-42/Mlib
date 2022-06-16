#include "Create_Keep_Offset_From_Movable.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Keep_Offset_From_Movable.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(FOLLOWER);
DECLARE_OPTION(FOLLOWED);
DECLARE_OPTION(OFFSET_X);
DECLARE_OPTION(OFFSET_Y);
DECLARE_OPTION(OFFSET_Z);

LoadSceneUserFunction CreateKeepOffsetFromMovable::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*keep_offset_from_movable"
        "\\s+follower=([\\w+-.]+)"
        "\\s+followed=([\\w+-.]+)"
        "\\s+offset=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateKeepOffsetFromMovable(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateKeepOffsetFromMovable::CreateKeepOffsetFromMovable(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateKeepOffsetFromMovable::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& follower_node = scene.get_node(match[FOLLOWER].str());
    auto& followed_node = scene.get_node(match[FOLLOWED].str());
    auto follower = std::make_shared<KeepOffsetFromMovable>(
        physics_engine.advance_times_,
        scene,
        match[FOLLOWER].str(),
        followed_node,
        followed_node.get_absolute_movable(),
        FixedArray<float, 3>{
            safe_stof(match[OFFSET_X].str()),
            safe_stof(match[OFFSET_Y].str()),
            safe_stof(match[OFFSET_Z].str())});
    linker.link_absolute_movable(follower_node, follower);
}
