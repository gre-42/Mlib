#include "Rigid_Body_Recorder_Gpx.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Integrator.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RigidBodyRecorderGpx::RigidBodyRecorderGpx(
    const std::string& filename,
    AdvanceTimes& advance_times,
    SceneNode& recorded_node,
    RigidBodyIntegrator* rbi,
    const TransformationMatrix<double, double, 3>* geographic_coordinates,
    const Focuses& focuses)
: focuses_{focuses},
  advance_times_{advance_times},
  recorded_node_{&recorded_node},
  rbi_{rbi},
  geographic_coordinates_{geographic_coordinates},
  track_writer_{filename},
  start_time_{std::chrono::steady_clock::now()}
{
    recorded_node_->clearing_observers.add(*this);
}

void RigidBodyRecorderGpx::advance_time(float dt) {
    if (recorded_node_ == nullptr) {
        return;
    }
    {
        std::shared_lock lock{focuses_.mutex};
        if (focuses_.countdown_active()) {
            return;
        }
    }
    if (geographic_coordinates_ == nullptr) {
        THROW_OR_ABORT("RigidBodyRecorderGpx::advance_time without geographic mapping");
    }
    track_writer_.write(geographic_coordinates_->transform(rbi_->abs_position().casted<double>()));
}

void RigidBodyRecorderGpx::notify_destroyed(const Object& destroyed_object) {
    rbi_ = nullptr;
    recorded_node_ = nullptr;
    advance_times_.schedule_delete_advance_time(*this);
}
