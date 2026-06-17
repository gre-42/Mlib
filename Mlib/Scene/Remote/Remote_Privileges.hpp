#pragma once
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <cstdint>

namespace Mlib {

enum class PositionFlags: uint32_t {
    NONE = 0,
    IS_REMOTELY_ACTIVATED_AVATAR = 1 << 0,
    IS_DEACTIVATED_AVATAR = 1 << 1,
    POSITION_CONTAINS_JUMP = 1 << 2,
    POSITION_IS_INCOMPLETE = 1 << 3
};

inline bool any(PositionFlags a) {
    return a != PositionFlags::NONE;
}

inline PositionFlags operator & (PositionFlags a, PositionFlags b) {
    return PositionFlags((uint32_t)a & (uint32_t)b);
}

inline PositionFlags& operator |= (PositionFlags& a, PositionFlags b) {
    (uint32_t&)a |= (uint32_t)b;
    return a;
}

struct PositionPrivileges {
    bool invalidate_transformation_history;
    bool update_position;
};

class RemotePrivileges {
public:
    RemotePrivileges(
        RemoteSiteId local_site,
        RemoteSiteId update_sender,
        RemoteSiteId object_owner,
        RemoteSiteId object_manager);
    bool is_manager_local;
    bool is_owner_local;
    bool is_owner_sender;
    PositionPrivileges position(PositionFlags flags);
};

}
