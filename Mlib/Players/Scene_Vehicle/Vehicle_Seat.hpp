#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

enum class VehicleSeat: uint32_t {
    DRIVER = 0,
    GUNNER = 1
};
static const size_t VEHICLE_SEAT_NBITS = 1;

VehicleSeat vehicle_seat_from_string(const std::string& seat);
std::string vehicle_seat_to_string(VehicleSeat seat);

}
