#pragma once
#include <cstdint>

namespace Mlib {

enum class ObjectCompression: uint32_t {
    NONE = 0xC0FEBABE,
    INCREMENTAL = 0x12345432
};

}
