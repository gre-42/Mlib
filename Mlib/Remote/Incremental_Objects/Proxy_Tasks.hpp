#pragma once

namespace Mlib {

enum class ProxyTasks {
    NONE = 0,
    SEND_LOCAL = 1 << 0,
    SEND_REMOTE = 1 << 1,
    SEND_OWNERSHIP = 1 << 2,
    RELOAD_SCENE = 1 << 3
};

inline bool any(ProxyTasks tasks) {
    return tasks != ProxyTasks::NONE;
}

inline ProxyTasks operator & (ProxyTasks a, ProxyTasks b) {
    return (ProxyTasks)((int)a & (int)b);
}

inline ProxyTasks operator | (ProxyTasks a, ProxyTasks b) {
    return (ProxyTasks)((int)a | (int)b);
}

inline ProxyTasks& operator |= (ProxyTasks& a, ProxyTasks b) {
    (int&)a |= (int)b;
    return a;
}

}
