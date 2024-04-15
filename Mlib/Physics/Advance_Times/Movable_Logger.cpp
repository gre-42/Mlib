#include "Movable_Logger.hpp"
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

MovableLogger::MovableLogger(
    DanglingRef<SceneNode> scene_node,
    StatusWriter& status_writer,
    StatusComponents log_components)
    : status_writer_{status_writer}
    , log_components_{log_components}
{
    scene_node->clearing_observers.add({ *this, CURRENT_SOURCE_LOCATION });
}

MovableLogger::~MovableLogger() {
    on_destroy.clear();
}

void MovableLogger::notify_destroyed(DanglingRef<SceneNode> destroyed_object) {
    global_object_pool.remove(this);
}

void MovableLogger::advance_time(float dt, std::chrono::steady_clock::time_point time) {
    status_writer_.write_status(std::cerr, log_components_);
}
