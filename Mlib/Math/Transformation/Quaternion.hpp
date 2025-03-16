#pragma once
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <cmath>
#include <iostream>

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

namespace Mlib {

template <class TRotation, class TPos, size_t tsize>
class OffsetAndTaitBryanAngles;

template <class TData>
class Quaternion {
public:
    static Quaternion identity() {
        return Quaternion{TData{1}, FixedArray<TData, 3>{TData{0}, TData{0}, TData{0}}};
    }
    Quaternion(Uninitialized)
        : v{ uninitialized }
    {}
    Quaternion(const TData& s, const FixedArray<TData, 3>& v)
        : v{ v }
        , s{ s }
    {}
    /**
     * From: https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
     * The case "trace <= 0" did not work properly.
     */
    // explicit Quaternion(const FixedArray<TData, 3, 3>& m) {
    //     TData trace = m(0, 0) + m(1, 1) + m(2, 2);
    //     if (trace > 0) {
    //         TData k = TData{0.5} / std::sqrt(trace + (TData)1);
    //         s = TData{0.25} / k;
    //         v(0) = (m(2, 1) - m(1, 2) ) * k;
    //         v(1) = (m(0, 2) - m(2, 0) ) * k;
    //         v(2) = (m(1, 0) - m(0, 1) ) * k;
    //     } else {
    //         if (m(0, 0) > m(1, 1) && m(0, 0) > m(2, 2) ) {
    //             TData k = (TData)2 * std::sqrt(1 + m(0, 0) - m(1, 1) - m(2, 2));
    //             s = (m(2, 1) - m(1, 2) ) / k;
    //             v(0) = TData{0.25} * k;
    //             v(1) = (m(0, 1) + m(1, 0) ) / k;
    //             v(2) = (m(0, 2) + m(2, 0) ) / k;
    //         } else if (m(1, 1) > m(2, 2)) {
    //             TData k = (TData)2 * std::sqrt(1 + m(1, 1) - m(0, 0) - m(2, 2));
    //             s = (m(0, 2) - m(2, 0) ) / k;
    //             v(0) = (m(0, 1) + m(1, 0) ) / k;
    //             v(1) = TData{0.25} * k;
    //             v(2) = (m(1, 2) + m(2, 1) ) / k;
    //         } else {
    //             TData k = (TData)2 * std::sqrt(1 + m(2, 2) - m(0, 0) - m(1, 1));
    //             s = (m(1, 0) - m(0, 1) ) / k;
    //             v(0) = (m(0, 2) + m(2, 0) ) / k;
    //             v(1) = (m(1, 2) + m(2, 1) ) / k;
    //             v(2) = TData{0.25} * k;
    //         }
    //         *this /= length();
    //     }
    // }
    /**
    * From: https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
    * Christian's alternative method
    * Does not work properly
    */
    // explicit Quaternion(const FixedArray<TData, 3, 3>& m) {
    //     s = std::sqrt(std::max<TData>(0, 1 + m(0, 0) + m(1, 1) + m(2, 2))) / 2;
    //     v(0) = std::sqrt(std::max<TData>(0, 1 + m(0, 0) - m(1, 1) - m(2, 2))) / 2;
    //     v(1) = std::sqrt(std::max<TData>(0, 1 - m(0, 0) + m(1, 1) - m(2, 2))) / 2;
    //     v(2) = std::sqrt(std::max<TData>(0, 1 - m(0, 0) - m(1, 1) + m(2, 2))) / 2;
    // 
    //     v(0) = std::copysign(v(0), m(2, 1) - m(1, 2));
    //     v(1) = std::copysign(v(1), m(0, 2) - m(2, 0));
    //     v(2) = std::copysign(v(2), m(1, 0) - m(0, 1));
    // }
    explicit Quaternion(const FixedArray<TData, 3, 3>& m)
        : Quaternion{ from_tait_bryan_angles(matrix_2_tait_bryan_angles(m)) }
    {}
    Quaternion(const FixedArray<TData, 3>& axis, const TData& angle)
        : v{ axis * std::sin(angle / TData{2}) }
        , s{ std::cos(angle / TData{2}) }
    {}
    static Quaternion<TData> from_tait_bryan_angles(const FixedArray<TData, 3>& angles) {
        return Quaternion{FixedArray<TData, 3>{(TData)0, (TData)0, (TData)1}, angles(2)} *
               Quaternion{FixedArray<TData, 3>{(TData)0, (TData)1, (TData)0}, angles(1)} *
               Quaternion{FixedArray<TData, 3>{(TData)1, (TData)0, (TData)0}, angles(0)};
    }
    /**
     * From: https://en.wikipedia.org/wiki/Slerp
     */
    Quaternion slerp(Quaternion q1, const TData& t) const {
        const Quaternion& q0 = *this;
        // Only unit quaternions are valid rotations.
        // Normalize to avoid undefined behavior.
        assert(std::abs(q0.length2() - 1) < 1e-6);
        assert(std::abs(q1.length2() - 1) < 1e-6);

        // Compute the cosine of the angle between the two vectors.
        TData dot = q0.dot0d(q1);

        // If the dot product is negative, slerp won't take
        // the shorter path. Note that q1 and -q1 are equivalent when
        // the negation is applied to all four components. Fix by 
        // reversing one quaternion.
        if (dot < 0.0f) {
            q1 = -q1;
            dot = -dot;
        }

        TData DOT_THRESHOLD = TData(0.9995);
        if (dot > DOT_THRESHOLD) {
            // If the inputs are too close for comfort, linearly interpolate
            // and normalize the result.
            Quaternion result = q0 + (q1 - q0) * t;
            result /= result.length();
            return result;
        }

        // Since dot is in range [0, DOT_THRESHOLD], acos is safe
        TData theta_0 = std::acos(dot);        // theta_0 = angle between input vectors
        TData theta = theta_0 * t;        // theta = angle between q0 and result
        TData sin_theta = std::sin(theta);     // compute this value only once
        TData sin_theta_0 = std::sin(theta_0); // compute this value only once

        TData s0 = std::cos(theta) - dot * sin_theta / sin_theta_0;  // == sin(theta_0 - theta) / sin(theta_0)
        TData s1 = sin_theta / sin_theta_0;

        return q0 * s0 + q1 * s1;
    }
    /**
     * From: https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
     */
    // The handling of singularities (std::abs(sinp) >= 1) seems to produce wrong results.
    // Using the version from "euclideanspace.com" instead.
    // FixedArray<TData, 3> to_tait_bryan_angles() const {
    //     FixedArray<TData, 3> angles;
    // 
    //     // roll (x-axis rotation)
    //     TData sinr_cosp = 2 * (s * v(0) + v(1) * v(2));
    //     TData cosr_cosp = 1 - 2 * (v(0) * v(0) + v(1) * v(1));
    //     angles(0) = std::atan2(sinr_cosp, cosr_cosp);
    // 
    //     // pitch (y-axis rotation)
    //     TData sinp = 2 * (s * v(1) - v(2) * v(0));
    //     if (std::abs(sinp) >= 1)
    //         angles(1) = std::copysign(TData{ M_PI / 2 }, sinp); // use 90 degrees if out of range
    //     else
    //         angles(1) = std::asin(sinp);
    // 
    //     // yaw (z-axis rotation)
    //     TData siny_cosp = 2 * (s * v(2) + v(0) * v(1));
    //     TData cosy_cosp = 1 - 2 * (v(1) * v(1) + v(2) * v(2));
    //     angles(2) = std::atan2(siny_cosp, cosy_cosp);
    // 
    //     return angles;
    // }
    /**
     * From: https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/
     */
    // Does not work for large rotations about the z axis (probably the std::asin case).
    // FixedArray<TData, 3> to_tait_bryan_angles() const {
    //     TData heading;
    //     TData attitude;
    //     TData bank;
    //     const TData& x = vector()(0);
    //     const TData& y = vector()(1);
    //     const TData& z = vector()(2);
    //     const TData& w = scalar();
    //     TData test = x * y + z * w;
    //     if (test > 0.499) { // singularity at north pole
    //         heading = 2 * std::atan2(x, w);
    //         attitude = M_PI / 2;
    //         bank = 0;
    //         return FixedArray<TData, 3>{ bank, heading, attitude };
    //     }
    //     if (test < -0.499) { // singularity at south pole
    //         heading = -2 * std::atan2(x, w);
    //         attitude = - M_PI / 2;
    //         bank = 0;
    //         return FixedArray<TData, 3>{ bank, heading, attitude };
    //     }
    //     TData sqx = x * x;
    //     TData sqy = y * y;
    //     TData sqz = z * z;
    //     heading = std::atan2(2 * y * w - 2 * x * z , 1 - 2 * sqy - 2 * sqz);
    //     attitude = std::asin(2 * test);
    //     bank = std::atan2(2 * x * w - 2 * y * z, 1 - 2 * sqx - 2 * sqz);
    //     return FixedArray<TData, 3>{ bank, heading, attitude };
    // }
    FixedArray<TData, 3> to_tait_bryan_angles() const {
        return matrix_2_tait_bryan_angles(to_rotation_matrix());
    }
    FixedArray<TData, 3, 3> to_rotation_matrix() const {
        auto x = rotate(FixedArray<TData, 3>{1.f, 0.f, 0.f});
        auto y = rotate(FixedArray<TData, 3>{0.f, 1.f, 0.f});
        auto z = rotate(FixedArray<TData, 3>{0.f, 0.f, 1.f});
        return FixedArray<TData, 3, 3>::init(
            x(0), y(0), z(0),
            x(1), y(1), z(1),
            x(2), y(2), z(2));
    }
    TData length2() const {
        return squared(s) + sum(squared(v));
    }
    TData length() const {
        return std::sqrt(length2());
    }
    Quaternion& operator /= (const TData& x) {
        s /= x;
        v /= x;
        return *this;
    }
    /**
     * From: https://www.geeks3d.com/20141201/how-to-rotate-a-vertex-by-a-quaternion-in-glsl/
     */
    FixedArray<TData, 3> rotate(const FixedArray<TData, 3>& p) const {
        return p + TData{2} * cross(v, cross(v, p) + s * p);
    }
    Quaternion inverse() const {
        assert(std::abs(length2() - 1) < 1e-6);
        return Quaternion{s, -v};
    }
    Quaternion operator * (const Quaternion& rhs) const {
        // From: https://www.sciencedirect.com/topics/computer-science/quaternion-multiplication
        return Quaternion{
            s * rhs.s - ::Mlib::dot0d(v, rhs.v),
            s * rhs.v + rhs.s * v + cross(v, rhs.v)};
    }
    Quaternion operator - (const Quaternion& rhs) const {
        return Quaternion{s - rhs.s, v - rhs.v};
    }
    Quaternion operator + (const Quaternion& rhs) const {
        return Quaternion{s + rhs.s, v + rhs.v};
    }
    Quaternion operator * (const TData& rhs) const {
        return Quaternion{s * rhs, v * rhs};
    }
    Quaternion operator - () const {
        return Quaternion{-s, -v};
    }
    TData dot0d(const Quaternion& rhs) const {
        return s * rhs.s + ::Mlib::dot0d(v, rhs.v);
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(v);
        archive(s);
    }
    // Storing s after v makes it possible to access the
    // quaternion with xyzw in shaders.
    FixedArray<TData, 3> v;
    TData s;
};

template <class TDir, class TPos>
class OffsetAndQuaternion {
public:
    static OffsetAndQuaternion identity() {
        return OffsetAndQuaternion{
            fixed_zeros<TPos, 3>(),
            Quaternion<TDir>::identity()};
    }
    static OffsetAndQuaternion<TDir, TPos> from_tait_bryan_angles(
        const OffsetAndTaitBryanAngles<TDir, TPos, 3>& a)
    {
        return OffsetAndQuaternion<TDir, TPos>{
            a.position,
            Quaternion<TDir>::from_tait_bryan_angles(a.rotation)};
    }
    OffsetAndQuaternion(Uninitialized)
        : t{ uninitialized }
        , q{ uninitialized }
    {}
    OffsetAndQuaternion(const FixedArray<TPos, 3>& t, const Quaternion<TDir>& q)
        : t{ t }
        , q{ q }
    {}
    explicit OffsetAndQuaternion(const FixedArray<TDir, 4, 4>& m)
        : t{ t3_from_4x4(m) }
        , q{ R3_from_4x4(m) }
    {}
    FixedArray<TPos, 3> transform(const FixedArray<TPos, 3>& p) const {
        return t + q.rotate(p);
    }
    OffsetAndQuaternion inverse() const {
        OffsetAndQuaternion result = uninitialized;
        result.q = q.inverse();
        result.t = -result.q.rotate(t);
        return result;
    }
    OffsetAndQuaternion operator * (const OffsetAndQuaternion& rhs) const {
        return OffsetAndQuaternion{
            transform(rhs.t),
            q * rhs.q};
    }
    template <class TAlpha>
    OffsetAndQuaternion slerp(const OffsetAndQuaternion& other, const TAlpha& a0) const {
        const OffsetAndQuaternion& m0 = *this;
        const OffsetAndQuaternion& m1 = other;
        return OffsetAndQuaternion{
            m0.t * TPos(1 - a0) + m1.t * TPos(a0),
            m0.q.slerp(m1.q, TDir(a0))};
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(t);
        archive(q);
    }
    FixedArray<TPos, 3> t;
    Quaternion<TDir> q;
};

template <class TData>
std::ostream& operator << (std::ostream& ostr, const Quaternion<TData>& q) {
    ostr << "s: " << q.s << ", v: " << q.v;
    return ostr;
}

template <class TDir, class TPos>
std::ostream& operator << (std::ostream& ostr, const OffsetAndQuaternion<TDir, TPos>& oq) {
    ostr << "t: " << oq.t << ", q: " << oq.q;
    return ostr;
}

}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
