#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <iosfwd>

namespace Mlib {

#pragma GCC push_options
#pragma GCC optimize ("O3")

struct ColoredVertex {
    FixedArray<float, 3> position;
    FixedArray<float, 3> color;
    FixedArray<float, 2> uv;
    FixedArray<float, 3> normal;
    FixedArray<float, 3> tangent;

    ColoredVertex transformed(const FixedArray<float, 4, 4>& m) const {
        return ColoredVertex{
            .position = dehomogenized_3(dot1d(m, homogenized_4(position))),
            .color = color,
            .uv = uv,
            .normal = dot1d(R3_from_4x4(m), normal),
            .tangent = dot1d(R3_from_4x4(m), tangent)};
    }
};

#pragma GCC pop_options

inline std::ostream& operator << (std::ostream& ostr, const ColoredVertex& v) {
    ostr << "p " << v.position << " n " << v.normal << " c " << v.color << " t " << v.uv;
    return ostr;
}

}
