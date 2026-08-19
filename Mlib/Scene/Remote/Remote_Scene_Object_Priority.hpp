#pragma once
#include <cstdint>
#include <vector>

namespace Mlib {

namespace RemoteSceneObjectPriority {
    static const int32_t REMOTE_USERS = 2;
    static const int32_t PLAYER = 0;
    static const int32_t RIGID_BODY_VEHICLE = 1;
    static const int32_t COUNTDOWN = 3;
    static const int32_t GAME_STATISTICS = -1;
}

namespace FullTransmissionMask {
    static const uint32_t REMOTE_USERS = 1 << 0;
    static const uint32_t PLAYER = 1 << 1;
    static const uint32_t RIGID_BODY_VEHICLE = 1 << 2;
    static const uint32_t COUNTDOWN = 1 << 3;
    static const uint32_t GAME_STATISTICS = 1 << 4;
    std::vector<uint32_t> transmission_lut();
}

}
