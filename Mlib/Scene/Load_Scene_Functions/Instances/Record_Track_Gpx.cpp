#include "Record_Track_Gpx.hpp"
#include <Mlib/Macro_Line_Executor.hpp>
#include <Mlib/Physics/Advance_Times/Rigid_Body_Recorder_Gpx.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene/User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>

using namespace Mlib;

LoadSceneUserFunction RecordTrackGpx::user_function = [](const LoadSceneUserFunctionArgs& args)
{
    static DECLARE_REGEX(regex,
        "^\\s*record_track_gpx"
        "\\s+node=([\\w+-.]+)"
        "\\s+filename=([\\w+-. \\(\\)/\\\\:]+)$");
    std::smatch match;
    if (Mlib::re::regex_match(args.line, match, regex)) {
        RecordTrackGpx(args.renderable_scene()).execute(match, args);
        return true;
    } else {
        return false;
    }
};

RecordTrackGpx::RecordTrackGpx(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RecordTrackGpx::execute(
    const std::smatch& match,
    const LoadSceneUserFunctionArgs& args)
{
    auto recorder_node = scene.get_node(match[1].str());
    auto rb = dynamic_cast<RigidBodyVehicle*>(recorder_node->get_absolute_movable());
    if (rb == nullptr) {
        throw std::runtime_error("Absolute movable is not a rigid body");
    }
    physics_engine.advance_times_.add_advance_time(std::make_shared<RigidBodyRecorderGpx>(
        args.fpath(match[2].str()).path,
        physics_engine.advance_times_,
        recorder_node,
        &rb->rbi_,
        scene_node_resources.get_geographic_mapping("world"),
        args.ui_focus.focuses));
}
