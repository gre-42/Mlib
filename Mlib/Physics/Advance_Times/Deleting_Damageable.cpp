#include "Deleting_Damageable.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>

using namespace Mlib;

DeletingDamageable::DeletingDamageable(
    Scene& scene,
    AdvanceTimes& advance_times,
    const std::string& root_node_name,
    float health,
    std::recursive_mutex& mutex)
: scene_{scene},
  advance_times_{advance_times},
  root_node_name_{root_node_name},
  health_{health},
  mutex_{mutex}
{
    scene_.get_node(root_node_name_)->add_destruction_observer(this);
}

void DeletingDamageable::notify_destroyed(void* obj) {
    advance_times_.schedule_delete_advance_time(this);
}

void DeletingDamageable::advance_time(float dt) {
    if (health_ <= 0) {
        std::lock_guard lock{mutex_};
        scene_.delete_root_node(root_node_name_);
    }
}

void DeletingDamageable::write_status(std::ostream& ostr, unsigned int log_components) const {
    if (log_components & STATUS_HEALTH) {
        ostr << "HP: " << health_ << std::endl;
    }
}

float DeletingDamageable::health() const {
    return health_;
}

void DeletingDamageable::damage(float amount) {
    health_ -= amount;
}
