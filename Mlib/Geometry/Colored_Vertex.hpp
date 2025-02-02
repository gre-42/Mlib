#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Bone_Weight.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Stats/Clamped.hpp>
#include <cstdint>
#include <iosfwd>
#include <vector>

namespace Mlib {

namespace Colors {

static const FixedArray<uint8_t, 4> BLACK{ (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)255 };
static const FixedArray<uint8_t, 4> WHITE{ (uint8_t)255, (uint8_t)255, (uint8_t)255, (uint8_t)255 };

static const FixedArray<uint8_t, 4> RED{ (uint8_t)255, (uint8_t)0, (uint8_t)0, (uint8_t)255 };
static const FixedArray<uint8_t, 4> GREEN{ (uint8_t)0, (uint8_t)255, (uint8_t)0, (uint8_t)255 };
static const FixedArray<uint8_t, 4> BLUE{ (uint8_t)0, (uint8_t)0, (uint8_t)255, (uint8_t)255 };

static const FixedArray<uint8_t, 4> PURPLE{ (uint8_t)255, (uint8_t)0, (uint8_t)255, (uint8_t)255 };
static const FixedArray<uint8_t, 4> CYAN{ (uint8_t)0, (uint8_t)255, (uint8_t)255, (uint8_t)255 };
static const FixedArray<uint8_t, 4> YELLOW{ (uint8_t)255, (uint8_t)255, (uint8_t)0, (uint8_t)255 };

inline uint8_t from_float(float f) {
    return (uint8_t)std::clamp(std::round(f * 255.f), 0.f, 255.f);
}

inline float to_float(uint8_t c) {
    return (float)c / 255.f;
}

inline uint8_t scale(uint8_t c, float s) {
    return (uint8_t)std::clamp(std::round((float)c * s), 0.f, 255.f);
}

inline FixedArray<uint8_t, 4> scale(const FixedArray<uint8_t, 4>& c, float s) {
    return {
        scale(c(0), s),
        scale(c(1), s),
        scale(c(2), s),
        c(3) };
}

inline FixedArray<uint8_t, 4> from_rgb(const FixedArray<float, 3>& rgb) {
    return {
        from_float(rgb(0)),
        from_float(rgb(1)),
        from_float(rgb(2)),
        (uint8_t)255 };
}

inline FixedArray<float, 3> to_rgb(const FixedArray<uint8_t, 4>& rgba) {
    return {
        to_float(rgba(0)),
        to_float(rgba(1)),
        to_float(rgba(2)) };
}

inline FixedArray<uint8_t, 4> from_rgba(const FixedArray<float, 4>& rgba) {
    return clamped(round(rgba * 255.f), 0.f, 255.f).casted<uint8_t>();
}

inline FixedArray<uint8_t, 4> multiply(const FixedArray<uint8_t, 4>& a, const FixedArray<uint8_t, 4>& b) {
    return clamped(round(a.casted<float>() / 255.f * b.casted<float>()), 0.f, 255.f).casted<uint8_t>();
}

}

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
    FixedArray<uint8_t, 4> color;
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
        const FixedArray<uint8_t, 4>& color = fixed_full<uint8_t, 4>(255),
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
    ostr << "p " << v.position << " n " << v.normal << " c " << v.color.template casted<uint16_t>() << " t " << v.uv;
    return ostr;
}

}
