#include "Playback_Winner_Track.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Rigid_Body_Playback.hpp>
#include <Mlib/Physics/Containers/Race_History.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

#define BEGIN_OPTIONS static size_t option_id = 1
#define DECLARE_OPTION(a) static const size_t a = option_id++

BEGIN_OPTIONS;
DECLARE_OPTION(NODE);
DECLARE_OPTION(SPEED);
DECLARE_OPTION(RANK);

LoadSceneUserFunction PlaybackWinnerTrack::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*playback_winner_track"
        "\\s+node=([\\w+-.]+)"
        "\\s+speed=([\\w+-.]+)"
        "\\s+rank=(\\d+)$");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PlaybackWinnerTrack(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PlaybackWinnerTrack::PlaybackWinnerTrack(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlaybackWinnerTrack::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    size_t rank = safe_stoz(match[RANK].str());
    std::string filename = players.get_winner_track_filename(rank).m_filename;
    if (filename.empty()) {
        THROW_OR_ABORT("Winner with rank " + std::to_string(rank) + " does not exist");
    }
    auto& playback_node = scene.get_node(match[NODE].str());
    auto playback = std::make_unique<RigidBodyPlayback>(
        filename,
        physics_engine.advance_times_,
        args.ui_focus.focuses,
        args.scene_node_resources.get_geographic_mapping("world.inverse"),
        safe_stof(match[SPEED].str()));
    linker.link_absolute_movable(playback_node, std::move(playback));
}
