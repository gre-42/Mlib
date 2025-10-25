#pragma once
#include <cstdint>

namespace Mlib {

enum class ObjectCompression: uint32_t {
    NONE,
    INCREMENTAL
};

}
