#pragma once
#include <cstdint>

namespace Mlib {

enum class RemoteSceneObjectType: uint32_t {
    REMOTE_USERS = 0xFE54321D,
    PLAYER = 0xC7A3C0FE,
    RIGID_BODY_CAR = 0xABC234DE,
    RIGID_BODY_AVATAR = 0x7AC4321F,
    // RIGID_BODY_HELICOPTER = 0xC4D9293B,  // this is currently the same as "car"
    // RIGID_BODY_PLANE = 0xFBD92927,       // this is currently the same as "car"
    COUNTDOWN = 0x3F9C2B4E
};

}
