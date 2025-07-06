#include "Vehicle_Spawners.hpp"
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

VehicleSpawners::VehicleSpawners() = default;

VehicleSpawners::~VehicleSpawners() = default;

VehicleSpawner& VehicleSpawners::get(const std::string& name) {
    auto it = spawners_.find(name);
    if (it == spawners_.end()) {
        THROW_OR_ABORT("Could not find vehicle spawner with name \"" + name + '"');
    }
    return *it->second;
}

void VehicleSpawners::set(const std::string& name, std::unique_ptr<VehicleSpawner>&& spawner) {
    if (!spawners_.try_emplace(name, std::move(spawner)).second) {
        THROW_OR_ABORT("Vehicle spawner with name \"" + name + "\" already exists");
    }
}

void VehicleSpawners::advance_time(float dt) {
    for (auto& s : spawners_) {
        s.second->advance_time(dt);
    }
}

size_t VehicleSpawners::nactive() const {
    size_t nactive = 0;
    for (const auto& [_, s] : spawners_) {
        nactive += s->has_scene_vehicle();
    }
    return nactive;
}
