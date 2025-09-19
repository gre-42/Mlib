#include "Playback_Winner_Track.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Rigid_Body_Playback.hpp>
#include <Mlib/Physics/Containers/Race_History.hpp>
#include <Mlib/Physics/Misc/Track_Element_File.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(suffix);
DECLARE_ARGUMENT(speed);
DECLARE_ARGUMENT(rank);
}

const std::string PlaybackWinnerTrack::key = "playback_winner_track";

LoadSceneJsonUserFunction PlaybackWinnerTrack::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    PlaybackWinnerTrack(args.physics_scene()).execute(args);
};

PlaybackWinnerTrack::PlaybackWinnerTrack(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void PlaybackWinnerTrack::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto rank = args.arguments.at<size_t>(KnownArgs::rank);
    auto wt = players.get_winner_track_filename(rank);
    if (!wt.has_value()) {
        THROW_OR_ABORT("Winner with rank " + std::to_string(rank) + " does not exist");
    }
    auto asset_id = args.arguments.at<std::string>(KnownArgs::asset_id);
    const auto& vars = args
        .asset_references["vehicles"]
        .at(asset_id)
        .rp;
    auto node_prefixes = vars.database.at<std::vector<std::string>>("node_prefixes");
    auto filename = wt->m_filename;
    auto playback = std::make_shared<RigidBodyPlayback>(
        std::make_unique<TrackElementFile>(create_ifstream(filename), filename),
        &countdown_start,
        scene_node_resources.get_geographic_mapping(VariableAndHash<std::string>{"world.inverse"}),
        args.arguments.at<float>(KnownArgs::speed),
        node_prefixes.size());
    physics_engine.advance_times_.add_advance_time({ *playback, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);

    auto suffix = args.arguments.at<std::string>(KnownArgs::suffix);
    for (const auto& [i, prefix] : enumerate(node_prefixes)) {
        DanglingBaseClassRef<SceneNode> node = scene.get_node(VariableAndHash<std::string>{prefix + suffix}, DP_LOC);
        node->clearing_pointers.add(playback);
        auto playback_object = playback->get_playback_object(i);
        node->set_absolute_movable(playback_object);
    }
}
