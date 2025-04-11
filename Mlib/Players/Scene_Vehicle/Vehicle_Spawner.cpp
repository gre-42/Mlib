#include "Vehicle_Spawner.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Spawn_Arguments.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SpawnVehicleAlreadySetBehavior Mlib::spawn_vehicle_already_set_behavior_from_string(
    const std::string& s)
{
    static const std::map<std::string, SpawnVehicleAlreadySetBehavior> m{
        {"throw", SpawnVehicleAlreadySetBehavior::THROW},
        {"update", SpawnVehicleAlreadySetBehavior::UPDATE}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown SpawnVehicleAlreadySetBehavior: \"" + s + '"');
    }
    return it->second;
}

VehicleSpawner::VehicleSpawner(Scene& scene, std::string suffix, std::string team_name)
    : scene_{ scene }
    , player_{ nullptr }
    , on_player_destroy_{ nullptr, CURRENT_SOURCE_LOCATION }
    , suffix_{ std::move(suffix) }
    , team_name_{ std::move(team_name) }
    , time_since_spawn_{ NAN }
    , time_since_deletion_{ 0.f }
    , spotted_by_vip_{ false }
    , respawn_cooldown_time_{ 0 }
{}

VehicleSpawner::~VehicleSpawner() = default;

void VehicleSpawner::notify_destroyed(const RigidBodyVehicle& rigid_body_vehicle) {
    size_t ndeleted = scene_vehicles_.remove_if([&rigid_body_vehicle](const std::unique_ptr<SceneVehicle>& v){
        return &v->rb() == &rigid_body_vehicle;
    });
    if (ndeleted != 1) {
        verbose_abort("Could not delete exactly one vehicle");
    }
    if (scene_vehicles_.empty()) {
        time_since_spawn_ = NAN;
        time_since_deletion_ = 0.f;
    }
}

DanglingBaseClassPtr<IPlayer> VehicleSpawner::player() {
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

void VehicleSpawner::set_player(
    const DanglingBaseClassRef<Player>& player,
    std::string role)
{
    if (player_ != nullptr) {
        THROW_OR_ABORT("Player already set");
    }
    player_ = player.ptr();
    on_player_destroy_.set(player_->on_destroy, CURRENT_SOURCE_LOCATION);
    on_player_destroy_.add([this]() { player_ = nullptr; }, CURRENT_SOURCE_LOCATION);
    role_ = std::move(role);
}

DanglingBaseClassRef<Player> VehicleSpawner::get_player() {
    if (player_ == nullptr) {
        THROW_OR_ABORT("Player not set");
    }
    return *player_;
}

float VehicleSpawner::get_respawn_cooldown_time() const {
    return respawn_cooldown_time_;
}

void VehicleSpawner::set_respawn_cooldown_time(float respawn_cooldown_time) {
    respawn_cooldown_time_ = respawn_cooldown_time;
}

float VehicleSpawner::get_time_since_deletion() const {
    return time_since_deletion_;
}

void VehicleSpawner::set_spawn_vehicle(
    SpawnVehicle spawn_vehicle,
    SpawnVehicleAlreadySetBehavior vehicle_spawner_already_set_behavior)
{
    if (spawn_vehicle_ &&
        (vehicle_spawner_already_set_behavior == SpawnVehicleAlreadySetBehavior::THROW))
    {
        THROW_OR_ABORT("Spawn vehicle function already set");
    }
    spawn_vehicle_ = std::move(spawn_vehicle);
}

bool VehicleSpawner::has_scene_vehicle() const {
    return !scene_vehicles_.empty();
}

SceneVehicle& VehicleSpawner::get_primary_scene_vehicle() {
    const VehicleSpawner* cthis = this;
    return const_cast<SceneVehicle&>(cthis->get_primary_scene_vehicle());
}

const SceneVehicle& VehicleSpawner::get_primary_scene_vehicle() const {
    if (scene_vehicles_.empty()) {
        THROW_OR_ABORT("Spawner has no scene vehicle");
    }
    return *scene_vehicles_.front();
}

const std::list<std::unique_ptr<SceneVehicle>>& VehicleSpawner::get_scene_vehicles() const {
    return scene_vehicles_;
}

void VehicleSpawner::set_scene_vehicles(std::list<std::unique_ptr<SceneVehicle>>&& scene_vehicles) {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    if (!scene_vehicles_.empty()) {
        THROW_OR_ABORT("Scene vehicles already set");
    }
    if (scene_vehicles.empty()) {
        THROW_OR_ABORT("Scene vehicles list is empty");
    }
    for (const auto& v : scene_vehicles) {
        if (v->scene_node_name().empty()) {
            THROW_OR_ABORT("Rigid body scene node name is empty");
        }
        if (v->scene_node()->shutting_down()) {
            THROW_OR_ABORT("Player received scene node that is shutting down");
        }
        if (scene_.root_node_scheduled_for_deletion(v->scene_node_name())) {
            THROW_OR_ABORT("Player received root node scheduled for deletion");
        }
    }
    scene_vehicles_ = std::move(scene_vehicles);
    for (const auto& v : scene_vehicles_) {
        v->rb().destruction_observers.add({ *this, CURRENT_SOURCE_LOCATION });
    }
    if (has_player()) {
        player_->set_vehicle_spawner(*this, role_);
    }
}

void VehicleSpawner::spawn(const SpawnPoint& spawn_point, CompressedScenePos spawn_y_offset) {
    if (has_player() && player_->has_scene_vehicle()) {
        THROW_OR_ABORT("Player \"" + player_->id() + "\" already has a vehicle before spawning");
    }
    if (!scene_vehicles_.empty()) {
        THROW_OR_ABORT("Scene vehicles already set before spawning");
    }
    if (!spawn_vehicle_) {
        THROW_OR_ABORT("Vehicle spawner not initialized");
    }
    auto spawn_args = SpawnArguments{
        .suffix = suffix_,
        .if_with_graphics = true,
        .if_with_physics = true,
        .y_offset = spawn_y_offset
    };
    spawn_vehicle_(spawn_point, spawn_args);
    if (scene_vehicles_.empty()) {
        THROW_OR_ABORT("Scene vehicles not set after spawning");
    }
    if (has_player() && (&player_->vehicle() != &get_primary_scene_vehicle())) {
        THROW_OR_ABORT("Player vehicle not set correctly after spawning");
    }
    notify_spawn();
}

void VehicleSpawner::delete_vehicle() {
    while (has_scene_vehicle()) {
        auto n = get_primary_scene_vehicle().scene_node_name();
        scene_.delete_root_node(n);
    }
}

void VehicleSpawner::notify_spawn() {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    if (scene_vehicles_.empty()) {
        THROW_OR_ABORT("Scene vehicles not set");
    }
    if (has_player()) {
        player_->single_waypoint_.notify_spawn();
    }
    time_since_spawn_ = 0.f;
    spotted_by_vip_ = false;
}

float VehicleSpawner::get_time_since_spawn() const {
    if (std::isnan(time_since_spawn_)) {
        THROW_OR_ABORT("Seconds since spawn requires previous call to notify_spawn");
    }
    return time_since_spawn_;
}

bool VehicleSpawner::get_spotted_by_vip() const {
    return spotted_by_vip_;
}

void VehicleSpawner::set_spotted_by_vip() {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    spotted_by_vip_ = true;
}

void VehicleSpawner::advance_time(float dt) {
    time_since_spawn_ += dt;
    time_since_deletion_ += dt;
}
