#include "Playback_Winner_Track.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Rigid_Body_Playback.hpp>
#include <Mlib/Physics/Containers/Race_History.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(speed);
DECLARE_ARGUMENT(rank);
}

const std::string PlaybackWinnerTrack::key = "playback_winner_track";

LoadSceneJsonUserFunction PlaybackWinnerTrack::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlaybackWinnerTrack(args.renderable_scene()).execute(args);
};

PlaybackWinnerTrack::PlaybackWinnerTrack(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlaybackWinnerTrack::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    size_t rank = args.arguments.at<size_t>(KnownArgs::rank);
    std::string filename = players.get_winner_track_filename(rank).m_filename;
    if (filename.empty()) {
        THROW_OR_ABORT("Winner with rank " + std::to_string(rank) + " does not exist");
    }
    auto& playback_node = scene.get_node(args.arguments.at(KnownArgs::node));
    auto playback = std::make_unique<RigidBodyPlayback>(
        filename,
        physics_engine.advance_times_,
        args.ui_focus.focuses,
        args.scene_node_resources.get_geographic_mapping("world.inverse"),
        args.arguments.at<float>(KnownArgs::speed));
    linker.link_absolute_movable(playback_node, std::move(playback));
}
