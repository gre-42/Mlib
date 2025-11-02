#pragma once
#include <cstdint>

namespace Mlib {

enum class RemoteSceneObjectType: uint32_t {
    REMOTE_USERS = 0xFE54321D,
    RIGID_BODY_VEHICLE = 0xABC234DE
};

}
