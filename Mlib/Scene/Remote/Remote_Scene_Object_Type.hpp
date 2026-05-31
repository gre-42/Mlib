#pragma once
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <cstdint>

namespace Mlib {

enum class RemoteSceneObjectType: RemoteSceneObjectUnderlyingType {
    REMOTE_USERS = 0xFE,
    PLAYER = 0xC7,
    RIGID_BODY_CAR = 0xAB,
    RIGID_BODY_AVATAR = 0x7A,
    // RIGID_BODY_HELICOPTER = 0xC4,  // this is currently the same as "car"
    // RIGID_BODY_PLANE = 0xFB,       // this is currently the same as "car"
    COUNTDOWN = 0x3F
};

inline RemoteSceneObjectType operator ~ (RemoteSceneObjectType v) {
    return (RemoteSceneObjectType)~(RemoteSceneObjectUnderlyingType)v;
}

}
