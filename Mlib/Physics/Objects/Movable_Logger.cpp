#include "Movable_Logger.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

using namespace Mlib;

MovableLogger::MovableLogger(
    SceneNode& scene_node,
    AdvanceTimes& advance_times,
    Loggable* logged,
    unsigned int log_components)
: scene_node_{scene_node},
  advance_times_{advance_times},
  logged_{logged},
  log_components_{log_components}
{
    scene_node.add_destruction_observer(this);
}

void MovableLogger::notify_destroyed(void* destroyed_object) {
    advance_times_.schedule_delete_advance_time(this);
}

void MovableLogger::advance_time(float dt) {
    logged_->log(std::cerr, log_components_);
}
