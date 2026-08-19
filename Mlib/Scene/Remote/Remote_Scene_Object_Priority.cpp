#include "Remote_Scene_Object_Priority.hpp"
#include <Mlib/Math/Log2.hpp>
#include <Mlib/Remote/Transmission_Scheduler.hpp>

using namespace Mlib;

std::vector<uint32_t> FullTransmissionMask::transmission_lut() {
    static auto lut = ::Mlib::transmission_lut(
        {
            int_log2(16),   // REMOTE_USERS
            int_log2(16),   // PLAYER
            int_log2(8),    // RIGID_BODY_VEHICLE
            int_log2(16),   // COUNTDOWN
            int_log2(16),   // GAME_STATISTICS
        },
        int_log2(64));
    return lut;
}
