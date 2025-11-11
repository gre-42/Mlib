#pragma once
#include <cstdint>

namespace Mlib {

enum class RemoteSceneObjectType: uint32_t {
    REMOTE_USERS = 0xFE54321D,
    PLAYER = 0xC7A3C0FE,
    RIGID_BODY_VEHICLE = 0xABC234DE
};

}
