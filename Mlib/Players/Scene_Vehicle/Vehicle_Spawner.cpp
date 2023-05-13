#include "Vehicle_Spawner.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

VehicleSpawner::VehicleSpawner(Scene& scene, const std::string& team_name)
: scene_{scene},
  scene_vehicle_{nullptr},
  player_{nullptr},
  team_name_{team_name},
  spotted_by_vip_{ false }
{}

VehicleSpawner::~VehicleSpawner() = default;

void VehicleSpawner::notify_vehicle_destroyed() {
    if (scene_vehicle_ == nullptr) {
        verbose_abort("Vehicle spawner has no vehicle");
    }
    scene_vehicle_ = nullptr;
}

IPlayer* VehicleSpawner::player() {
    return player_;
}

void VehicleSpawner::set_team_name(const std::string& team_name) {
    if (!team_name_.empty()) {
        THROW_OR_ABORT("Team already set");
    }
    team_name_ = team_name;
}

std::string VehicleSpawner::get_team_name() const {
    return team_name_;
}

bool VehicleSpawner::has_player() const {
    return (player_ != nullptr);
}

void VehicleSpawner::set_player(Player& player) {
    if (player_ != nullptr) {
        THROW_OR_ABORT("Player already set");
    }
    player_ = &player;
}

Player& VehicleSpawner::get_player() {
    if (player_ == nullptr) {
        THROW_OR_ABORT("Player not set");
    }
    return *player_;
}

void VehicleSpawner::set_spawn_vehicle(std::function<void(const SpawnPoint&)> spawn_vehicle) {
    if (spawn_vehicle_) {
        THROW_OR_ABORT("Spawn vehicle function already set");
    }
    spawn_vehicle_ = std::move(spawn_vehicle);
}

bool VehicleSpawner::has_scene_vehicle() const {
    return scene_vehicle_ != nullptr;
}

SceneVehicle& VehicleSpawner::get_scene_vehicle() {
    if (scene_vehicle_ == nullptr) {
        THROW_OR_ABORT("Scene vehicle not set");
    }
    return *scene_vehicle_;
}

void VehicleSpawner::set_scene_vehicle(std::unique_ptr<SceneVehicle>&& scene_vehicle) {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    if (scene_vehicle_ != nullptr) {
        THROW_OR_ABORT("Scene vehicle already set");
    }
    if (scene_vehicle->scene_node_name().empty()) {
        THROW_OR_ABORT("Rigid body scene node name is empty");
    }
    if (scene_vehicle->scene_node().shutting_down()) {
        THROW_OR_ABORT("Player received scene node that is shutting down");
    }
    if (scene_.root_node_scheduled_for_deletion(scene_vehicle->scene_node_name())) {
        THROW_OR_ABORT("Player received root node scheduled for deletion");
    }
    if (scene_vehicle->rb().spawner_ != nullptr) {
        THROW_OR_ABORT("Spawner already set");
    }
    scene_vehicle_ = std::move(scene_vehicle);
    scene_vehicle_->rb().spawner_ = this;
    if (has_player()) {
        player_->set_scene_vehicle(*scene_vehicle_);
    }
}

void VehicleSpawner::spawn(const SpawnPoint& spawn_point, double spawn_y_offset) {
    if (has_player() && player_->has_scene_vehicle()) {
        THROW_OR_ABORT("Player \"" + player_->name() + "\"already has a vehicle before spawning");
    }
    if (scene_vehicle_ != nullptr) {
        THROW_OR_ABORT("Scene vehicle already set before spawning");
    }
    SpawnPoint sp2 = spawn_point;
    sp2.position(1) += spawn_y_offset;
    spawn_vehicle_(sp2);
    if (scene_vehicle_ == nullptr) {
        THROW_OR_ABORT("Scene vehicle not set after spawning");
    }
    if (has_player() && (&player_->vehicle() != scene_vehicle_.get())) {
        THROW_OR_ABORT("Player vehicle not set after spawning");
    }
    notify_spawn();
}

void VehicleSpawner::notify_spawn() {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    if (scene_vehicle_ == nullptr) {
        THROW_OR_ABORT("Scene vehicle not set");
    }
    if (has_player()) {
        player_->single_waypoint_.notify_spawn();
    }
    spawn_time_ = std::chrono::steady_clock::now();
    spotted_by_vip_ = false;
}

float VehicleSpawner::seconds_since_spawn() const {
    scene_.delete_node_mutex().notify_reading();
    if (spawn_time_ == std::chrono::time_point<std::chrono::steady_clock>()) {
        THROW_OR_ABORT("Seconds since spawn requires previous call to notify_spawn");
    }
    return std::chrono::duration<float>(std::chrono::steady_clock::now() - spawn_time_).count();
}

bool VehicleSpawner::spotted_by_vip() const {
    scene_.delete_node_mutex().notify_reading();
    return spotted_by_vip_;
}

void VehicleSpawner::set_spotted_by_vip() {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    spotted_by_vip_ = true;
}
