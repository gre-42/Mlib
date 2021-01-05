#pragma once
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <cmath>
#include <iostream>

namespace Mlib {

template <class TData>
class Quaternion {
public:
    static Quaternion identity() {
        return Quaternion{1, FixedArray<TData, 3>{0, 0, 0}};
    }
    Quaternion()
    {}
    Quaternion(const TData& s, const FixedArray<TData, 3>& v)
    : v_{v},
      s_{s}
    {}
    /**
     * From: https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
     */
    explicit Quaternion(const FixedArray<TData, 3, 3>& m) {
        TData trace = m(0, 0) + m(1, 1) + m(2, 2);
        if (trace > 0) {
            TData k = 0.5 / std::sqrt(trace + 1);
            s_ = 0.25 / k;
            v_(0) = (m(2, 1) - m(1, 2) ) * k;
            v_(1) = (m(0, 2) - m(2, 0) ) * k;
            v_(2) = (m(1, 0) - m(0, 1) ) * k;
        } else {
            if (m(0, 0) > m(1, 1) && m(0, 0) > m(2, 2) ) {
                TData k = 2 * std::sqrt(1 + m(0, 0) - m(1, 1) - m(2, 2));
                s_ = (m(2, 1) - m(1, 2) ) / k;
                v_(0) = 0.25 * k;
                v_(1) = (m(0, 1) + m(1, 0) ) / k;
                v_(2) = (m(0, 2) + m(2, 0) ) / k;
            } else if (m(1, 1) > m(2, 2)) {
                TData k = 2 * std::sqrt(1 + m(1, 1) - m(0, 0) - m(2, 2));
                s_ = (m(0, 2) - m(2, 0) ) / k;
                v_(0) = (m(0, 1) + m(1, 0) ) / k;
                v_(1) = 0.25 * k;
                v_(2) = (m(1, 2) + m(2, 1) ) / k;
            } else {
                TData k = 2 * std::sqrt(1 + m(2, 2) - m(0, 0) - m(1, 1));
                s_ = (m(1, 0) - m(0, 1) ) / k;
                v_(0) = (m(0, 2) + m(2, 0) ) / k;
                v_(1) = (m(1, 2) + m(2, 1) ) / k;
                v_(2) = 0.25 * k;
            }
        }
    }
    /**
     * From: https://en.wikipedia.org/wiki/Slerp
     */
    Quaternion slerp(Quaternion q1, const TData& t) {
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

        TData DOT_THRESHOLD = 0.9995;
        if (dot > DOT_THRESHOLD) {
            // If the inputs are too close for comfort, linearly interpolate
            // and normalize the result.
            Quaternion result = q0 + (q1 - q0) * t;
            result /= result.length();
            return result;
        }

        // Since dot is in range [0, DOT_THRESHOLD], acos is safe
        TData theta_0 = acos(dot);        // theta_0 = angle between input vectors
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
    FixedArray<TData, 3> to_tait_bryan_angles() const {
        FixedArray<TData, 3> angles;

        // roll (x-axis rotation)
        TData sinr_cosp = 2 * (s_ * v_(0) + v_(1) * v_(2));
        TData cosr_cosp = 1 - 2 * (v_(0) * v_(0) + v_(1) * v_(1));
        angles(0) = std::atan2(sinr_cosp, cosr_cosp);

        // pitch (y-axis rotation)
        TData sinp = 2 * (s_ * v_(1) - v_(2) * v_(0));
        if (std::abs(sinp) >= 1)
            angles(1) = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
        else
            angles(1) = std::asin(sinp);

        // yaw (z-axis rotation)
        TData siny_cosp = 2 * (s_ * v_(2) + v_(0) * v_(1));
        TData cosy_cosp = 1 - 2 * (v_(1) * v_(1) + v_(2) * v_(2));
        angles(2) = std::atan2(siny_cosp, cosy_cosp);

        return angles;
    }
    TData length2() const {
        return squared(s_) + sum(squared(v_));
    }
    TData length() const {
        return std::sqrt(length2());
    }
    Quaternion operator /= (const TData& x) {
        s_ /= x;
        v_ /= x;
        return *this;
    }
    /**
     * From: https://www.geeks3d.com/20141201/how-to-rotate-a-vertex-by-a-quaternion-in-glsl/
     */
    FixedArray<TData, 3> rotate(const FixedArray<TData, 3>& p) const {
        return p + TData{2} * cross(v_, cross(v_, p) + s_ * p);
    }
    Quaternion inverse() const {
        assert(std::abs(length2() - 1) < 1e-6);
        return Quaternion{s_, -v_};
    }
    Quaternion operator * (const Quaternion& rhs) const {
        // From: https://www.sciencedirect.com/topics/computer-science/quaternion-multiplication
        return Quaternion{
            s_ * rhs.s_ - ::Mlib::dot0d(v_, rhs.v_),
            s_ * rhs.v_ + rhs.s_ * v_ + cross(v_, rhs.v_)};
    }
    Quaternion operator - (const Quaternion& rhs) const {
        return Quaternion{s_ - rhs.s_, v_ - rhs.v_};
    }
    Quaternion operator + (const Quaternion& rhs) const {
        return Quaternion{s_ + rhs.s_, v_ + rhs.v_};
    }
    Quaternion operator * (const TData& rhs) const {
        return Quaternion{s_ * rhs, v_ * rhs};
    }
    Quaternion operator - () const {
        return Quaternion{-s_, -v_};
    }
    TData dot0d(const Quaternion& rhs) const {
        return s_ * rhs.s_ + ::Mlib::dot0d(v_, rhs.v_);
    }
    const TData& scalar() const {
        return s_;
    }
    TData& scalar() {
        return s_;
    }
    const FixedArray<TData, 3>& vector() const {
        return v_;
    }
    FixedArray<TData, 3>& vector() {
        return v_;
    }
private:
    // Storing s_ after v_ makes it possible to access the
    // quaternion with xyzw in shaders.
    FixedArray<TData, 3> v_;
    TData s_;
};

template <class TData>
class OffsetAndQuaternion {
public:
    static OffsetAndQuaternion identity() {
        return OffsetAndQuaternion{
            fixed_zeros<float, 3>(),
            Quaternion<TData>::identity()};
    }
    OffsetAndQuaternion()
    {}
    OffsetAndQuaternion(const FixedArray<TData, 3>& o, const Quaternion<TData>& q)
    : o_{o},
      q_{q}
    {}
    explicit OffsetAndQuaternion(const FixedArray<float, 4, 4>& m)
    : o_{t3_from_4x4(m)},
      q_{R3_from_4x4(m)}
    {}
    FixedArray<TData, 3> transform(const FixedArray<TData, 3>& p) const {
        return o_ + q_.rotate(p);
    }
    OffsetAndQuaternion inverse() const {
        OffsetAndQuaternion result;
        result.q_ = q_.inverse();
        result.o_ = -result.q_.rotate(o_);
        return result;
    }
    OffsetAndQuaternion operator * (const OffsetAndQuaternion& rhs) const {
        return OffsetAndQuaternion{
            transform(rhs.o_),
            q_ * rhs.q_};
    }
    const FixedArray<TData, 3>& offset() const {
        return o_;
    }
    FixedArray<TData, 3>& offset() {
        return o_;
    }
    const Quaternion<TData>& quaternion() const {
        return q_;
    }
    Quaternion<TData>& quaternion() {
        return q_;
    }
private:
    FixedArray<TData, 3> o_;
    Quaternion<TData> q_;
};

template <class TData>
std::ostream& operator << (std::ostream& ostr, const Quaternion<TData>& q) {
    ostr << "s: " << q.scalar() << ", v: " << q.vector();
    return ostr;
}

template <class TData>
std::ostream& operator << (std::ostream& ostr, const OffsetAndQuaternion<TData>& oq) {
    ostr << "o: " << oq.offset() << ", q: " << oq.quaternion();
    return ostr;
}

}
