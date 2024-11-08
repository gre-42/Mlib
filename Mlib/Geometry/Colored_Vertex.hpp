#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Bone_Weight.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <iosfwd>
#include <vector>

namespace Mlib {

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

enum class ColoredVertexFeatures {
    NONE = 0,
    POSITION = 1 << 0,
    COLOR = 1 << 1,
    UV = 1 << 2,
    NORMAL = 1 << 3,
    TANGENT = 1 << 4
};

inline bool any(ColoredVertexFeatures f) {
    return f != ColoredVertexFeatures::NONE;
}

inline ColoredVertexFeatures operator & (ColoredVertexFeatures a, ColoredVertexFeatures b) {
    return (ColoredVertexFeatures)((int)a & (int)b);
}

inline ColoredVertexFeatures& operator |= (ColoredVertexFeatures& a, ColoredVertexFeatures b) {
    return (ColoredVertexFeatures&)((int&)a |= (int)b);
}

template <class TPos>
struct ColoredVertex {
    FixedArray<TPos, 3> position;
    FixedArray<float, 3> color;
    FixedArray<float, 2> uv;
    FixedArray<float, 3> normal;
    FixedArray<float, 3> tangent;

    ColoredVertex(Uninitialized)
        : position{ uninitialized }
        , color{ uninitialized }
        , uv{ uninitialized }
        , normal{ uninitialized }
        , tangent{ uninitialized }
    {}
    ColoredVertex(
        const FixedArray<TPos, 3>& position,
        const FixedArray<float, 3>& color = fixed_ones<float, 3>(),
        const FixedArray<float, 2>& uv = fixed_zeros<float, 2>(),
        const FixedArray<float, 3>& normal = fixed_zeros<float, 3>(),
        const FixedArray<float, 3>& tangent = fixed_zeros<float, 3>())
        : position{ position }
        , color{ color }
        , uv{ uv }
        , normal{ normal }
        , tangent{ tangent }
    {}
    template <class TResultPos>
    ColoredVertex<TResultPos> casted() const {
        return ColoredVertex<TResultPos>{
            position.template casted<TResultPos>(),
            color,
            uv,
            normal,
            tangent}; 
    }
    template <class TResultPos>
    explicit operator ColoredVertex<TResultPos>() const {
        return casted<TResultPos>();
    }
    ColoredVertex translated(const FixedArray<TPos, 3>& shift) const {
        return ColoredVertex{
            position + shift,
            color,
            uv,
            normal,
            normal};
    }
    ColoredVertex rotated(const FixedArray<float, 3, 3>& m) const {
        return ColoredVertex{
            dot1d(m.casted<TPos>(), position),
            color,
            uv,
            dot1d(m, normal),
            dot1d(m, tangent)};
    }
    ColoredVertex scaled(float scale) const {
        return ColoredVertex{
            (TPos)scale * position,
            color,
            uv,
            normal,
            tangent};
    }
    ColoredVertex transformed(
        const TransformationMatrix<float, TPos, 3>& m,
        const FixedArray<float, 3, 3>& r) const
    {
        return ColoredVertex{
            m.transform(position),
            color,
            uv,
            dot1d(r, normal),
            dot1d(r, tangent)};
    }
    ColoredVertex transformed(
        const std::vector<BoneWeight>& weights,
        const UUVector<OffsetAndQuaternion<float, TPos>>& oqs) const
    {
        ColoredVertex result{
            fixed_zeros<TPos, 3>(),
            color,
            uv,
            fixed_zeros<float, 3>(),
            fixed_zeros<float, 3>()};
        for (const BoneWeight& w : weights) {
            assert(w.bone_index < oqs.size());
            const OffsetAndQuaternion<float, TPos>& oq = oqs[w.bone_index];
            result.position += w.weight * oq.transform(position);
            result.normal += w.weight * oq.q.rotate(normal);
            result.tangent += w.weight * oq.q.rotate(tangent);
        }
        result.normal /= std::sqrt(sum(squared(result.normal)));
        result.tangent /= std::sqrt(sum(squared(result.tangent)));
        return result;
    }
    template <class TOperation>
    ColoredVertex transformed_uv(const TOperation& m) const {
        return ColoredVertex{
            position,
            color,
            m(uv),
            normal,
            tangent};
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
