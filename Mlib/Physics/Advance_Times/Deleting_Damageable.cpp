#include "Deleting_Damageable.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
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
    , shutting_down_{ false }
    , node_on_clear_{ scene_.get_node(root_node_name_, DP_LOC)->on_clear }
    , rb_on_destroy_{ rb_->on_destroy }
{
    if (rb_->damageable_ != nullptr) {
        THROW_OR_ABORT("Rigid body already has a damageable");
    }
    rb_->damageable_ = this;
    dgs_.add([this]() { if (rb_ != nullptr) { rb_->damageable_ = nullptr; } });
    advance_times_.add_advance_time(*this);
    dgs_.add([this]() { advance_times_.delete_advance_time(*this, CURRENT_SOURCE_LOCATION); });
    node_on_clear_.add([this]() { if (!shutting_down_) { delete this; } });
    rb_on_destroy_.add([this]() { rb_ = nullptr; if (!shutting_down_) { delete this; } });
}

DeletingDamageable::~DeletingDamageable() {
    shutting_down_ = true;
}

void DeletingDamageable::advance_time(float dt) {
    if (delete_node_when_health_leq_zero_ && (health() <= 0)) {
        scene_.schedule_delete_root_node(root_node_name_);
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
