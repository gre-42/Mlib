#include "Vehicle_Spawners.hpp"
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iomanip>

using namespace Mlib;

VehicleSpawners::VehicleSpawners()
    : spawners_{"Spawner"}
{}

VehicleSpawners::~VehicleSpawners() = default;

VehicleSpawner& VehicleSpawners::get(const VariableAndHash<std::string>& name) {
    auto it = spawners_.find(name);
    if (it == spawners_.end()) {
        THROW_OR_ABORT("Could not find vehicle spawner with name \"" + *name + '"');
    }
    return *it->second;
}

void VehicleSpawners::set(const VariableAndHash<std::string>& name, std::unique_ptr<VehicleSpawner>&& spawner) {
    if (!spawners_.try_emplace(name, std::move(spawner)).second) {
        THROW_OR_ABORT("Vehicle spawner with name \"" + *name + "\" already exists");
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

void VehicleSpawners::print_status() const {
    for (const auto& [n, s] : spawners_) {
        std::stringstream sstr;
        sstr << "Spawner " << std::left << std::setw(15) << std::string("\"" + *n + "\"") << ": active=" << (int)s->has_scene_vehicle();
        if (s->has_player()) {
            sstr << ", parking=" << (int)s->get_player()->is_parking()
                << ", pedestrian=" << (int)s->get_player()->is_pedestrian();
        }
        linfo() << sstr.str();
    }
}
