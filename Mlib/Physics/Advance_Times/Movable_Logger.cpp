#include "Movable_Logger.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

MovableLogger::MovableLogger(
    SceneNode& scene_node,
    AdvanceTimes& advance_times,
    StatusWriter* status_writer,
    StatusComponents log_components)
: scene_node_{scene_node},
  advance_times_{advance_times},
  status_writer_{status_writer},
  log_components_{log_components}
{
    scene_node.destruction_observers.add(*this);
}

void MovableLogger::notify_destroyed(const Object& destroyed_object) {
    advance_times_.schedule_delete_advance_time(*this);
}

void MovableLogger::advance_time(float dt) {
    status_writer_->write_status(std::cerr, log_components_);
}
