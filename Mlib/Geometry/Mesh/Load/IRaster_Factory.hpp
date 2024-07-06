#pragma once
#include <cstdint>
#include <memory>

namespace Mlib {

namespace Dff {

struct Image;
class IRaster;

class IRasterFactory {
public:
    virtual bool is_p8_supported() const = 0;
    virtual std::unique_ptr<IRaster> create_raster(const Image& img, uint32_t type) const = 0;
    virtual std::unique_ptr<IRaster> create_raster(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t type,
        uint32_t platform,
        uint32_t compression,
        uint32_t num_levels,
        bool has_alpha,
        const uint8_t* palette) const = 0;
};

}

}
