#include "Drivers.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

Drivers::Drivers() = default;

Drivers::~Drivers() = default;

void Drivers::set_roles(std::vector<std::string> roles) {
    roles_ = std::move(roles);
    roles_set_ = std::set(roles_.begin(), roles_.end());
}

bool Drivers::role_exists(const std::string& role) const {
    return roles_set_.contains(role);
}

bool Drivers::role_is_free(const std::string& role) {
    if (!roles_set_.contains(role)) {
        THROW_OR_ABORT("Unknown role: \"" + role + '"');
    }
    return !players_.contains(role);
}

void Drivers::add(
    std::string role,
    DanglingBaseClassRef<IPlayer> player,
    SourceLocation loc)
{
    if (!roles_set_.contains(role)) {
        THROW_OR_ABORT("Unknown role: \"" + role + '"');
    }
    auto it = players_.try_emplace(std::move(role), player, player->on_clear_vehicle(), loc);
    if (!it.second) {
        THROW_OR_ABORT("Player with role \"" + role + "\" already exists");
    }
    it.first->second.on_destroy(
        [this, it](){ players_.erase(it.first); },
        CURRENT_SOURCE_LOCATION);
}

DanglingBaseClassPtr<IPlayer> Drivers::try_get(const std::string& role) const {
    if (!roles_set_.contains(role)) {
        THROW_OR_ABORT("Unknown role: \"" + role + '"');
    }
    auto it = players_.find(role);
    if (it == players_.end()) {
        return nullptr;
    }
    return it->second.object().ptr();
}

void Drivers::clear() {
    clear_container_recursively(players_);
}

const std::string* Drivers::first_free_role() const {
    for (const auto& role : roles_) {
        if (!players_.contains(role)) {
            return &role;
        }
    }
    return nullptr;
}

const std::string* Drivers::next_free_role(const std::string& current_role) const {
    bool found = false;
    for (const auto& role : roles_) {
        if (found) {
            if (!players_.contains(role)) {
                return &role;
            }
        } else {
            found |= (role == current_role);
        }
    }
    return nullptr;
}

const Drivers::PlayersMap& Drivers::players_map() const {
    return players_;
}

