#pragma once
#include <cstdint>

namespace Mlib {

enum class TransmittedFields: uint32_t {
    NONE = 0,
    SITE_ID = 1 << 0,
    END = 1 << 1
};

inline bool any(TransmittedFields tasks) {
    return tasks != TransmittedFields::NONE;
}

inline TransmittedFields operator & (TransmittedFields a, TransmittedFields b) {
    return (TransmittedFields)((int)a & (int)b);
}

inline TransmittedFields operator | (TransmittedFields a, TransmittedFields b) {
    return (TransmittedFields)((int)a | (int)b);
}

inline TransmittedFields& operator |= (TransmittedFields& a, TransmittedFields b) {
    (int&)a |= (int)b;
    return a;
}

inline TransmittedFields operator ~ (TransmittedFields a) {
    return (TransmittedFields)(~(uint32_t)a);
}

}
