#include "Vehicle_Seat.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

VehicleSeat Mlib::vehicle_seat_from_string(const std::string& seat) {
    static const std::map<std::string, VehicleSeat> m{
        {"driver", VehicleSeat::DRIVER},
        {"gunner", VehicleSeat::GUNNER}
    };
    auto it = m.find(seat);
    if (it == m.end()) {
        throw std::runtime_error("Unknown seat: \"" + seat + '"');
    }
    return it->second;
}

std::string Mlib::vehicle_seat_to_string(VehicleSeat seat) {
    switch (seat) {
    case VehicleSeat::DRIVER:
        return "driver";
    case VehicleSeat::GUNNER:
        return "gunner";
    }
    throw std::runtime_error("Unknown seat: " + std::to_string((uint32_t)seat));
}
