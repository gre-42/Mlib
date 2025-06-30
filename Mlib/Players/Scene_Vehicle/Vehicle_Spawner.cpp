#include "Vehicle_Spawner.hpp"
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

SpawnTrigger Mlib::spawn_trigger_from_string(const std::string& s) {
    static const std::map<std::string, SpawnTrigger> m{
        {"none", SpawnTrigger::NONE},
        {"bystanders", SpawnTrigger::BYSTANDERS},
        {"team_deathmatch", SpawnTrigger::TEAM_DEATHMATCH}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown SpawnTrigger: \"" + s + '"');
    }
    return it->second;
}

VehicleSpawner::VehicleSpawner(
    Scene& scene,
    std::string suffix,
    std::string team_name,
    std::string group_name,
    SpawnTrigger spawn_trigger)
    : scene_{ scene }
    , player_{ nullptr }
    , on_player_destroy_{ nullptr, CURRENT_SOURCE_LOCATION }
    , suffix_{ std::move(suffix) }
    , team_name_{ std::move(team_name) }
    , group_name_{ std::move(group_name) }
    , spawn_trigger_{ spawn_trigger }
    , time_since_spawn_{ NAN }
    , time_since_deletion_{ 0.f }
    , spotted_by_vip_{ false }
    , respawn_cooldown_time_{ 0 }
{}

VehicleSpawner::~VehicleSpawner() {
    if (!scene_vehicles_.empty()) {
        verbose_abort("~VehicleSpawner: Scene vehicles remaining");
    }
}

DanglingBaseClassPtr<IPlayer> VehicleSpawner::player() {
    return player_;
}

std::string VehicleSpawner::get_team_name() const {
    return team_name_;
}

std::string VehicleSpawner::get_group_name() const {
    return group_name_;
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
    TrySpawnVehicle try_spawn_vehicle,
    SpawnVehicleAlreadySetBehavior vehicle_spawner_already_set_behavior)
{
    if (try_spawn_vehicle_ &&
        (vehicle_spawner_already_set_behavior == SpawnVehicleAlreadySetBehavior::THROW))
    {
        THROW_OR_ABORT("Spawn vehicle function already set");
    }
    try_spawn_vehicle_ = std::move(try_spawn_vehicle);
}

bool VehicleSpawner::has_scene_vehicle() const {
    return !scene_vehicles_.empty();
}

DanglingBaseClassRef<SceneVehicle> VehicleSpawner::get_primary_scene_vehicle() {
    if (scene_vehicles_.empty()) {
        THROW_OR_ABORT("Spawner has no scene vehicle");
    }
    return scene_vehicles_.front().object();
}

DanglingBaseClassRef<const SceneVehicle> VehicleSpawner::get_primary_scene_vehicle() const {
    return const_cast<VehicleSpawner*>(this)->get_primary_scene_vehicle();
}

const DanglingList<SceneVehicle>& VehicleSpawner::get_scene_vehicles() const {
    return scene_vehicles_;
}

void VehicleSpawner::set_scene_vehicles(
    std::list<std::unique_ptr<SceneVehicle, DeleteFromPool<SceneVehicle>>>&& scene_vehicles)
{
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    if (!scene_vehicles_.empty()) {
        THROW_OR_ABORT("Scene vehicles already set");
    }
    if (scene_vehicles.empty()) {
        THROW_OR_ABORT("Scene vehicles list is empty");
    }
    for (const auto& v : scene_vehicles) {
        if (v->scene_node_name()->empty()) {
            THROW_OR_ABORT("Rigid body scene node name is empty");
        }
        if (v->scene_node()->shutting_down()) {
            THROW_OR_ABORT("Player received scene node that is shutting down");
        }
        if (scene_.root_node_scheduled_for_deletion(v->scene_node_name())) {
            THROW_OR_ABORT("Player received root node scheduled for deletion");
        }
    }
    for (auto& l : scene_vehicles) {
        auto res = scene_vehicles_.emplace_back(
            DanglingBaseClassRef<SceneVehicle>{*l, CURRENT_SOURCE_LOCATION},
            CURRENT_SOURCE_LOCATION);
        l.release();
        res->on_destroy([this](){
            if (scene_vehicles_.empty()) {
                time_since_spawn_ = NAN;
                time_since_deletion_ = 0.f;
            }
        }, CURRENT_SOURCE_LOCATION);
    }
    if (has_player()) {
        player_->set_vehicle_spawner(*this, role_);
    }
}

bool VehicleSpawner::try_spawn(
    const TransformationMatrix<SceneDir, CompressedScenePos, 3>& spawn_point,
    const GeometrySpawnArguments& geometry)
{
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    if (!try_spawn_vehicle_) {
        THROW_OR_ABORT("Vehicle spawner not initialized");
    }
    if (geometry.action == SpawnAction::DRY_RUN) {
        return try_spawn_vehicle_(spawn_point, geometry, nullptr);
    }
    if (geometry.action != SpawnAction::DO_IT) {
        verbose_abort("Unknown spawn action: " + std::to_string((int)geometry.action));
    }
    if (has_player() && player_->has_scene_vehicle()) {
        THROW_OR_ABORT("Player \"" + player_->id() + "\" already has a vehicle before spawning");
    }
    if (!scene_vehicles_.empty()) {
        THROW_OR_ABORT("Scene vehicles already set before spawning");
    }
    auto node_args = NodeSpawnArguments{
        .suffix = suffix_,
        .if_with_graphics = true,
        .if_with_physics = true
    };
    if (!try_spawn_vehicle_(spawn_point, geometry, &node_args)) {
        if (!scene_vehicles_.empty()) {
            verbose_abort("Scene vehicles set after failed spawning");
        }
        if (has_player() && player_->has_scene_vehicle()) {
            verbose_abort("Player \"" + player_->id() + "\" has a vehicle after failed spawning");
        }
        return false;
    } else {
        if (scene_vehicles_.empty()) {
            verbose_abort("Scene vehicles not set after spawning");
        }
        if (has_player() && (&player_->vehicle().get() != &get_primary_scene_vehicle().get())) {
            verbose_abort("Player vehicle not set correctly after spawning");
        }
        notify_spawn();
        return true;
    }
}

void VehicleSpawner::delete_vehicle() {
    scene_.delete_node_mutex().assert_this_thread_is_deleter_thread();
    while (has_scene_vehicle()) {
        auto n = get_primary_scene_vehicle()->scene_node_name();
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

SpawnTrigger VehicleSpawner::get_spawn_trigger() const {
    return spawn_trigger_;
}

void VehicleSpawner::advance_time(float dt) {
    time_since_spawn_ += dt;
    time_since_deletion_ += dt;
}
