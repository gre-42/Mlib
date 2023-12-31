#include "Movable_Logger.hpp"
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

MovableLogger::MovableLogger(
    DanglingRef<SceneNode> scene_node,
    AdvanceTimes& advance_times,
    StatusWriter& status_writer,
    StatusComponents log_components)
: advance_times_{advance_times},
  status_writer_{status_writer},
  log_components_{log_components}
{
    scene_node->clearing_observers.add(*this);
}

void MovableLogger::notify_destroyed(DanglingRef<const SceneNode> destroyed_object) {
    advance_times_.schedule_delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}

void MovableLogger::advance_time(float dt) {
    status_writer_.write_status(std::cerr, log_components_);
}
