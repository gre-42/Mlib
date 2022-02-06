#include "Playback_Track.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Rigid_Body_Playback.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

LoadSceneUserFunction PlaybackTrack::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*playback_track"
        "\\s+node=([\\w+-.]+)"
        "\\s+speed=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        PlaybackTrack(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

PlaybackTrack::PlaybackTrack(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlaybackTrack::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto playback_node = scene.get_node(match[1].str());
    auto playback = std::make_shared<RigidBodyPlayback>(
        args.fpath(match[3].str()).path,
        physics_engine.advance_times_,
        args.ui_focus.focuses,
        safe_stof(match[2].str()));
    linker.link_absolute_movable(*playback_node, playback);
}
