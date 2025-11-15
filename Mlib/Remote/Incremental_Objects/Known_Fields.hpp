#pragma once
#include <cstdint>

namespace Mlib {

enum class KnownFields: uint32_t {
    NONE = 0,
    END = 1 << 1
};

inline bool any(KnownFields fields) {
    return fields != KnownFields::NONE;
}

inline KnownFields operator & (KnownFields a, KnownFields b) {
    return (KnownFields)((int)a & (int)b);
}

inline KnownFields operator | (KnownFields a, KnownFields b) {
    return (KnownFields)((int)a | (int)b);
}

inline KnownFields& operator |= (KnownFields& a, KnownFields b) {
    (int&)a |= (int)b;
    return a;
}

}
