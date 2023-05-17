#include "Playback_Track.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Rigid_Body_Playback.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(speed);
DECLARE_ARGUMENT(filename);
}

const std::string PlaybackTrack::key = "playback_track";

LoadSceneJsonUserFunction PlaybackTrack::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlaybackTrack(args.renderable_scene()).execute(args);
};

PlaybackTrack::PlaybackTrack(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlaybackTrack::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& playback_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    auto playback = std::make_unique<RigidBodyPlayback>(
        args.arguments.path(KnownArgs::filename),
        physics_engine.advance_times_,
        args.ui_focus.focuses,
        scene_node_resources.get_geographic_mapping("world.inverse"),
        args.arguments.at<float>(KnownArgs::speed));
    linker.link_absolute_movable(playback_node, std::move(playback));
}
