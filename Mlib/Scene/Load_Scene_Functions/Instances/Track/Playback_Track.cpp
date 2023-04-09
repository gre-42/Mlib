#include "Playback_Track.hpp"
#include <Mlib/FPath.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Rigid_Body_Playback.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/Linker.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

const std::string PlaybackTrack::key = "playback_track";

LoadSceneUserFunction PlaybackTrack::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^node=([\\w+-.]+)"
        "\\s+speed=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)$");
    Mlib::re::smatch match;
    if (!Mlib::re::regex_match(args.line, match, regex)) {
        THROW_OR_ABORT("Could not parse user function arguments");
    }
    PlaybackTrack(args.renderable_scene()).execute(match, args);
};

PlaybackTrack::PlaybackTrack(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void PlaybackTrack::execute(
    const Mlib::re::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    Linker linker{ physics_engine.advance_times_ };
    auto& playback_node = scene.get_node(match[1].str());
    auto playback = std::make_unique<RigidBodyPlayback>(
        args.fpath(match[3].str()).path,
        physics_engine.advance_times_,
        args.ui_focus.focuses,
        args.scene_node_resources.get_geographic_mapping("world.inverse"),
        safe_stof(match[2].str()));
    linker.link_absolute_movable(playback_node, std::move(playback));
}
