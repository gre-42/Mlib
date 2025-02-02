#include "Playback_Track.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Macro_Executor/Asset_Group_Replacement_Parameters.hpp>
#include <Mlib/Macro_Executor/Asset_References.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Rigid_Body_Playback.hpp>
#include <Mlib/Physics/Misc/Track_Element_File.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Absolute_Movable_Setter.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(asset_id);
DECLARE_ARGUMENT(suffix);
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
    auto asset_id = args.arguments.at<std::string>(KnownArgs::asset_id);
    const auto& vars = args
        .asset_references["vehicles"]
        .at(asset_id)
        .rp;
    auto node_prefixes = vars.database.at<std::vector<std::string>>("NODE_PREFIXES");
    auto filename = args.arguments.path(KnownArgs::filename);
    auto playback = std::make_shared<RigidBodyPlayback>(
        std::make_unique<TrackElementFile>(create_ifstream(filename), filename),
        args.ui_focus.focuses,
        scene_node_resources.get_geographic_mapping("world.inverse"),
        args.arguments.at<float>(KnownArgs::speed),
        node_prefixes.size());
    physics_engine.advance_times_.add_advance_time({ *playback, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);

    auto suffix = args.arguments.at<std::string>(KnownArgs::suffix);
    for (const auto& [i, prefix] : enumerate(node_prefixes)) {
        DanglingRef<SceneNode> node = scene.get_node(prefix + suffix, DP_LOC);
        node->clearing_pointers.add(playback);
        auto playback_object = playback->get_playback_object(i);
        node->set_absolute_movable(playback_object);
    }
}
