#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <iosfwd>

namespace Mlib {

struct ColoredVertex {
    FixedArray<float, 3> position;
    FixedArray<float, 3> color;
    FixedArray<float, 2> uv;
    FixedArray<float, 3> normal;
};

inline std::ostream& operator << (std::ostream& ostr, const ColoredVertex& v) {
    ostr << "p " << v.position << " n " << v.normal << " c " << v.color << " t " << v.uv;
    return ostr;
}

}
