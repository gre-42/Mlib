#pragma once
#include <cstdint>

namespace Mlib {

namespace RemoteSceneObjectPriority {
    static const int32_t REMOTE_USERS = 2;
    static const int32_t PLAYER = 0;
    static const int32_t RIGID_BODY_VEHICLE = 1;
    static const int32_t COUNTDOWN = 3;
}

namespace FullTransmissionRemainder {
    static const uint32_t REMOTE_USERS = 0;
    static const uint32_t PLAYER = 0;
    static const uint32_t RIGID_BODY_VEHICLE = 1;
    static const uint32_t COUNTDOWN = 0;
}

}
