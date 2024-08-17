#pragma once
#include <Mlib/Geometry/Mesh/Load/IRaster.hpp>

namespace Mlib {
namespace Dff {

class Palette;

namespace Ps2Flags {
    enum Flags: uint32_t {
        NEWSTYLE  = 0x1,    // has GIF tags and transfer DMA chain
        SWIZZLED8 = 0x2,
        SWIZZLED4 = 0x4
    };
};

class IRasterPs2 : public IRaster {
public:
    virtual uint32_t& flags() = 0;
    virtual uint32_t type() const = 0;
    virtual uint32_t pixel_size() const = 0;
    virtual uint8_t* lock_palette(uint32_t lock_mode) = 0;
    virtual void unlock_palette() = 0;
};
}
}
