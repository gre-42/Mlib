#include "Deleting_Damageable.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

DeletingDamageable::DeletingDamageable(
    Scene& scene,
    AdvanceTimes& advance_times,
    std::string root_node_name,
    float health,
    bool delete_node_when_health_leq_zero)
    : scene_{ scene }
    , advance_times_{ advance_times }
    , root_node_name_{ std::move(root_node_name) }
    , health_{ health }
    , delete_node_when_health_leq_zero_{ delete_node_when_health_leq_zero }
    , rb_{ &get_rigid_body_vehicle(scene.get_node(root_node_name_, DP_LOC)) }
    , node_on_clear_{ scene_.get_node(root_node_name_, DP_LOC)->on_clear, CURRENT_SOURCE_LOCATION }
    , rb_on_destroy_{ rb_->on_destroy, CURRENT_SOURCE_LOCATION }
{
    if (rb_->damageable_ != nullptr) {
        THROW_OR_ABORT("Rigid body already has a damageable");
    }
    rb_->damageable_ = this;
    dgs_.add([this]() { if (rb_ != nullptr) { rb_->damageable_ = nullptr; } });
    advance_times_.add_advance_time({ *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    node_on_clear_.add([this]() { global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
    rb_on_destroy_.add([this]() { rb_ = nullptr; global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

DeletingDamageable::~DeletingDamageable() {
    on_destroy.clear();
}

void DeletingDamageable::advance_time(float dt, const StaticWorld& world) {
    if (delete_node_when_health_leq_zero_ && (health() <= 0)) {
        scene_.schedule_delete_root_node(root_node_name_);
    }
}

void DeletingDamageable::write_status(std::ostream& ostr, StatusComponents log_components, const StaticWorld& world) const {
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

StatusWriter& DeletingDamageable::child_status_writer(const std::vector<VariableAndHash<std::string>>& name) {
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
