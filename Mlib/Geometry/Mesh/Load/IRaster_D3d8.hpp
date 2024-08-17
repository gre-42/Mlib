#pragma once
#include <Mlib/Geometry/Mesh/Load/IRaster.hpp>

namespace Mlib {
namespace Dff {

class Palette;

class IRasterD3d8 : public IRaster {
public:
    virtual uint32_t type() const = 0;
    virtual uint8_t* palette() = 0;
};
}
}
