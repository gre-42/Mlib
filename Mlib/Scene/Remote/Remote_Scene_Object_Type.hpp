#pragma once
#include <cstdint>

namespace Mlib {

enum class RemoteSceneObjectType: uint32_t {
    REMOTE_USERS = 0xFE54321D,
    PLAYER = 0xC7A3C0FE,
    RIGID_BODY_CAR = 0xABC234DE,
    RIGID_BODY_AVATAR = 0x7AC4321F
};

}
