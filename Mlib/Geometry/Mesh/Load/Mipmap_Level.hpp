#pragma once
#include <cstdint>
#include <vector>

namespace Mlib {
namespace Dff {

enum class AllocationMode {
    ALLOCATE,
    NO_ALLOCATE
};

struct MipmapLevel {
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    std::vector<uint8_t> data;
    inline uint32_t size() const {
        return stride * height;
    }
    static std::vector<MipmapLevel> compute(
        uint32_t width,
        uint32_t height,
        uint32_t stride,
        uint32_t internal_format,
        uint32_t max_num_levels,
        AllocationMode allocation_mode);
};

}
}
