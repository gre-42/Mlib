#pragma once
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Bone_Weight.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Quaternion.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <iosfwd>
#include <vector>

namespace Mlib {

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

template <class TPos>
struct ColoredVertex {
    FixedArray<TPos, 3> position;
    FixedArray<float, 3> color;
    FixedArray<float, 2> uv;
    FixedArray<float, 3> normal;
    FixedArray<float, 3> tangent;

    template <class TResultPos>
    ColoredVertex<TResultPos> casted() const {
        return ColoredVertex<TResultPos>{
            .position = position TEMPLATEV casted<TResultPos>(),
            .color = color,
            .uv = uv,
            .normal = normal,
            .tangent = tangent}; 
    }
    ColoredVertex rotated(const FixedArray<float, 3, 3>& m) const {
        return ColoredVertex{
            .position = dot1d(m.casted<TPos>(), position),
            .color = color,
            .uv = uv,
            .normal = dot1d(m, normal),
            .tangent = dot1d(m, tangent)};
    }
    ColoredVertex scaled(float scale) const {
        return ColoredVertex{
            .position = (TPos)scale * position,
            .color = color,
            .uv = uv,
            .normal = normal,
            .tangent = tangent};
    }
    ColoredVertex transformed(
        const TransformationMatrix<float, TPos, 3>& m,
        const FixedArray<float, 3, 3>& r) const
    {
        return ColoredVertex{
            .position = m.transform(position),
            .color = color,
            .uv = uv,
            .normal = dot1d(r, normal),
            .tangent = dot1d(r, tangent)};
    }
    ColoredVertex transformed(
        const std::vector<BoneWeight>& weights,
        const std::vector<OffsetAndQuaternion<float, TPos>>& oqs) const
    {
        ColoredVertex result{
            .position = fixed_zeros<TPos, 3>(),
            .color = color,
            .uv = uv,
            .normal = fixed_zeros<float, 3>(),
            .tangent = fixed_zeros<float, 3>()};
        for (const BoneWeight& w : weights) {
            assert(w.bone_index < oqs.size());
            const OffsetAndQuaternion<float, TPos>& oq = oqs[w.bone_index];
            result.position += w.weight * oq.transform(position);
            result.normal += w.weight * oq.quaternion().rotate(normal);
            result.tangent += w.weight * oq.quaternion().rotate(tangent);
        }
        result.normal /= std::sqrt(sum(squared(result.normal)));
        result.tangent /= std::sqrt(sum(squared(result.tangent)));
        return result;
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(position, color, uv, normal, tangent);
    }
};

#ifdef __GNUC__
    #pragma GCC pop_options
#endif

template <class TPos>
inline std::ostream& operator << (std::ostream& ostr, const ColoredVertex<TPos>& v) {
    ostr << "p " << v.position << " n " << v.normal << " c " << v.color << " t " << v.uv;
    return ostr;
}

}
