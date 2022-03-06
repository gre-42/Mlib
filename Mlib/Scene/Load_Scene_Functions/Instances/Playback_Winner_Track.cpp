#include "Playback_Winner_Track.hpp"
#include <Mlib/Physics/Advance_Times/Movables/Rigid_Body_Playback.hpp>
#include <Mlib/Physics/Containers/Game_History.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Strings/From_Number.hpp>

using namespace Mlib;

LoadSceneUserFunction PlaybackWinnerTrack::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*playback_winner_track"
        "\\s+node=([\\w+-.]+)"
        "\\s+speed=([\\w+-.]+)"
        "\\s+rank=(\\d+)$");
    std::smatch match;
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
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    size_t rank = safe_stoz(match[3].str());
    std::string filename = players.get_winner_track_filename(rank).m_filename;
    if (filename.empty()) {
        throw std::runtime_error("Winner with rank " + std::to_string(rank) + " does not exist");
    }
    auto& playback_node = scene.get_node(match[1].str());
    auto playback = std::make_shared<RigidBodyPlayback>(
        filename,
        physics_engine.advance_times_,
        args.ui_focus.focuses,
        safe_stof(match[2].str()));
    linker.link_absolute_movable(playback_node, playback);
}
