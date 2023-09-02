#include "Deleting_Damageable.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

DeletingDamageable::DeletingDamageable(
    Scene& scene,
    AdvanceTimes& advance_times,
    const std::string& root_node_name,
    float health,
    bool delete_node_when_health_leq_zero,
    DeleteNodeMutex& delete_node_mutex)
: scene_{scene},
  advance_times_{advance_times},
  root_node_name_{root_node_name},
  health_{health},
  delete_node_when_health_leq_zero_{delete_node_when_health_leq_zero},
  delete_node_mutex_{delete_node_mutex}
{
    scene_.get_node(root_node_name_, DP_LOC)->clearing_observers.add(*this);
}

void DeletingDamageable::notify_destroyed(DanglingRef<const SceneNode> destroyed_object) {
    advance_times_.schedule_delete_advance_time(*this, CURRENT_SOURCE_LOCATION);
}

void DeletingDamageable::advance_time(float dt) {
    if (delete_node_when_health_leq_zero_ && (health() <= 0)) {
        std::scoped_lock lock{ delete_node_mutex_ };
        scene_.delete_root_node(root_node_name_);
    }
}

void DeletingDamageable::write_status(std::ostream& ostr, StatusComponents log_components) const {
    if (log_components & StatusComponents::HEALTH) {
        ostr << "HP: " << health() << std::endl;
    }
}

float DeletingDamageable::get_value(StatusComponents log_components) const {
    if (log_components == StatusComponents::HEALTH) {
        return health();
    }
    THROW_OR_ABORT("Unknown status component: " + std::to_string((unsigned int)log_components));
}

StatusWriter& DeletingDamageable::child_status_writer(const std::vector<std::string>& name) {
    THROW_OR_ABORT("DeletingDamageable has no children");
}

float DeletingDamageable::health() const {
    std::shared_lock lock{health_mutex_};
    return health_;
}

void DeletingDamageable::damage(float amount) {
    std::scoped_lock lock{health_mutex_};
    health_ -= amount;
}
