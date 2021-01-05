#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/BoneWeight.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Quaternion.hpp>
#include <iosfwd>
#include <vector>

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
    ColoredVertex transformed(
        const std::vector<BoneWeight>& weights,
        const std::vector<OffsetAndQuaternion<float>>& oqs) const
    {
        ColoredVertex result{
            .position = fixed_zeros<float, 3>(),
            .color = color,
            .uv = uv,
            .normal = fixed_zeros<float, 3>(),
            .tangent = fixed_zeros<float, 3>()};
        for (const BoneWeight& w : weights) {
            assert(w.bone_index < oqs.size());
            const OffsetAndQuaternion<float>& oq = oqs[w.bone_index];
            result.position += w.weight * oq.transform(position);
            result.normal += w.weight * oq.quaternion().rotate(normal);
            result.tangent += w.weight * oq.quaternion().rotate(tangent);
        }
        result.normal /= std::sqrt(sum(squared(result.normal)));
        result.tangent /= std::sqrt(sum(squared(result.tangent)));
        return result;
    }
};

#pragma GCC pop_options

inline std::ostream& operator << (std::ostream& ostr, const ColoredVertex& v) {
    ostr << "p " << v.position << " n " << v.normal << " c " << v.color << " t " << v.uv;
    return ostr;
}

}
