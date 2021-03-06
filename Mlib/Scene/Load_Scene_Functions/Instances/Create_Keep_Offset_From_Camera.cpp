#include "Create_Keep_Offset_From_Camera.hpp"
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Advance_Times/Keep_Offset_From_Camera.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(FOLLOWER);
DECLARE_OPTION(OFFSET_X);
DECLARE_OPTION(OFFSET_Y);
DECLARE_OPTION(OFFSET_Z);

LoadSceneUserFunction CreateKeepOffsetFromCamera::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*keep_offset_from_camera"
        "\\s+follower=([\\w+-.]+)"
        "\\s+offset=([\\w+-.]+)\\s+([\\w+-.]+)\\s+([\\w+-.]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        CreateKeepOffsetFromCamera(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

CreateKeepOffsetFromCamera::CreateKeepOffsetFromCamera(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateKeepOffsetFromCamera::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& follower_node = scene.get_node(match[FOLLOWER].str());
    auto follower = std::make_shared<KeepOffsetFromCamera>(
        physics_engine.advance_times_,
        scene,
        selected_cameras,
        FixedArray<float, 3>{
            safe_stof(match[OFFSET_X].str()),
            safe_stof(match[OFFSET_Y].str()),
            safe_stof(match[OFFSET_Z].str())});
    linker.link_absolute_movable(follower_node, follower);
}
