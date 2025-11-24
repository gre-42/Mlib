#pragma once

namespace Mlib {

enum class PlayerSitePrivileges {
    NONE = 0,
    CONTROLLER = 1 << 0,
    MANAGER = 1 << 1
};

inline bool any(PlayerSitePrivileges p) {
    return p != PlayerSitePrivileges::NONE;
}

inline PlayerSitePrivileges operator & (PlayerSitePrivileges a, PlayerSitePrivileges b) {
    return (PlayerSitePrivileges)((int)a & (int)b);
}

inline PlayerSitePrivileges operator | (PlayerSitePrivileges a, PlayerSitePrivileges b) {
    return (PlayerSitePrivileges)((int)a | (int)b);
}

inline PlayerSitePrivileges& operator |= (PlayerSitePrivileges& a, PlayerSitePrivileges b) {
    (int&)a |= (int)b;
    return a;
}

}
