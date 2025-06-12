#include "Drivers.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

Drivers::Drivers() = default;

Drivers::~Drivers() = default;

void Drivers::set_seats(std::vector<std::string> seats) {
    seats_ = std::move(seats);
    seats_set_ = std::set(seats_.begin(), seats_.end());
}

bool Drivers::seat_exists(const std::string& seat) const {
    return seats_set_.contains(seat);
}

bool Drivers::seat_is_free(const std::string& seat) {
    if (!seats_set_.contains(seat)) {
        THROW_OR_ABORT("Unknown seat: \"" + seat + '"');
    }
    return !players_.contains(seat);
}

void Drivers::add(
    std::string seat,
    DanglingBaseClassRef<IPlayer> player,
    SourceLocation loc)
{
    if (!seats_set_.contains(seat)) {
        THROW_OR_ABORT("Unknown seat: \"" + seat + '"');
    }
    auto it = players_.try_emplace(std::move(seat), player, player->on_clear_vehicle(), loc);
    if (!it.second) {
        THROW_OR_ABORT("Player with seat \"" + seat + "\" already exists");
    }
    it.first->second.on_destroy(
        [this, it](){ players_.erase(it.first); },
        CURRENT_SOURCE_LOCATION);
}

DanglingBaseClassPtr<IPlayer> Drivers::try_get(const std::string& seat) const {
    if (!seats_set_.contains(seat)) {
        THROW_OR_ABORT("Unknown seat: \"" + seat + '"');
    }
    auto it = players_.find(seat);
    if (it == players_.end()) {
        return nullptr;
    }
    return it->second.object().ptr();
}

void Drivers::clear() {
    clear_container_recursively(players_);
}

const std::string* Drivers::first_free_seat() const {
    for (const auto& seat : seats_) {
        if (!players_.contains(seat)) {
            return &seat;
        }
    }
    return nullptr;
}

const std::string* Drivers::next_free_seat(const std::string& current_seat) const {
    bool found = false;
    for (const auto& seat : seats_) {
        if (found) {
            if (!players_.contains(seat)) {
                return &seat;
            }
        } else {
            found |= (seat == current_seat);
        }
    }
    return nullptr;
}

const Drivers::PlayersMap& Drivers::players_map() const {
    return players_;
}

