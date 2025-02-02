#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Physics/Actuators/Tire_Power_Intent.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <iosfwd>
#include <list>

namespace Mlib {

class RigidBodyVehicle;
class RigidBodyPulses;
class AttachedWheel;
struct PhysicsEngineConfig;

struct PointEqualityConstraint {
    FixedArray<ScenePos, 3> p0;
    FixedArray<ScenePos, 3> p1;
    float beta = 0.5;
    inline FixedArray<float, 3> v(float dt) const {
        return beta / dt * (p1 - p0).casted<float>();
    }
};

template <size_t ndim>
struct GenericLineEqualityConstraint {};

template <>
struct GenericLineEqualityConstraint<0> {
    GenericLineEqualityConstraint(PointEqualityConstraint pec) : pec{ std::move(pec) } {}
    PointEqualityConstraint pec;
};

template <>
struct GenericLineEqualityConstraint<1> {
    PointEqualityConstraint pec;
    FixedArray<float, 1, 3> null_space;
};

using LineEqualityConstraint = GenericLineEqualityConstraint<1>;

struct PlaneEqualityConstraint {
    PointEqualityConstraint pec;
    FixedArray<float, 3> plane_normal;
};

struct NormalImpulse {
    FixedArray<SceneDir, 3> normal;
    float lambda_total = 0;
};

// struct PlaneEqualityConstraint {
//     NormalImpulse normal_impulse;
//     float intercept;
//     float b = 0;
//     float beta = 0.5;
//     inline float C(const FixedArray<float, 3>& x) const {
//         return -(dot0d(normal_impulse.normal, x) + intercept);
//     }
//     inline float v(const FixedArray<float, 3>& p, float dt) const {
//         return b + beta / dt * C(p);
//     }
// };

struct PlaneInequalityConstraint {
    NormalImpulse normal_impulse;
    float overlap;
    float b = 0;
    float slop = 0;
    float beta = 0.02f;
    inline float bias() const {
        return std::max(0.f, overlap - slop);
    }
    inline float v(float dt) const {
        return b + beta / dt * bias();
    }
};

template <class TConstraint>
struct BoundedNormalConstraint1D {
    TConstraint constraint;
    float lambda_min = -INFINITY;
    float lambda_max = INFINITY;
    inline float clamped_lambda(float lambda) {
        lambda = std::clamp(constraint.normal_impulse.lambda_total + lambda, lambda_min, lambda_max) - constraint.normal_impulse.lambda_total;
        if (std::abs(lambda) > 1e6) {
            THROW_OR_ABORT("Lambda out of bounds");
        }
        constraint.normal_impulse.lambda_total += lambda;
        if (std::abs(constraint.normal_impulse.lambda_total) > 1e6) {
            THROW_OR_ABORT("Lambda-total out of bounds");
        }
        return lambda;
    }
};

template <class TConstraint>
struct BoundedFreeConstraint1D {
    TConstraint constraint;
    float lambda_total = 0.f;
    float lambda_min = -INFINITY;
    float lambda_max = INFINITY;
    inline float clamped_lambda(float lambda) {
        lambda = std::clamp(lambda_total + lambda, lambda_min, lambda_max) - lambda_total;
        if (std::abs(lambda) > 1e6) {
            THROW_OR_ABORT("Lambda out of bounds");
        }
        lambda_total += lambda;
        if (std::abs(lambda_total) > 1e6) {
            THROW_OR_ABORT("Lambda-total out of bounds");
        }
        return lambda;
    }
};

struct ShockAbsorberConstraint {
    NormalImpulse normal_impulse;
    float fit;
    float distance;
    float Ks;  // K_spring
    float Ka;  // K_absorber
};

using BoundedPlaneEqualityConstraint = BoundedFreeConstraint1D<PlaneEqualityConstraint>;
using BoundedPlaneInequalityConstraint = BoundedNormalConstraint1D<PlaneInequalityConstraint>;
using BoundedShockAbsorberConstraint = BoundedNormalConstraint1D<ShockAbsorberConstraint>;

class IContactInfo {
public:
    virtual ~IContactInfo() = default;
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) = 0;
    virtual void finalize() {}
};

template <size_t tnullspace>
class GenericLineContactInfo1: public IContactInfo {
public:
    GenericLineContactInfo1(
        RigidBodyPulses& rbp0,
        const FixedArray<float, 3>& v1,
        const GenericLineEqualityConstraint<tnullspace>& lec);
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) override;
private:
    RigidBodyPulses& rbp0_;
    FixedArray<float, 3> v1_;
    GenericLineEqualityConstraint<tnullspace> lec_;
};

using PointContactInfo1 = GenericLineContactInfo1<0>;
using LineContactInfo1 = GenericLineContactInfo1<1>;

template <size_t tnullspace>
class GenericLineContactInfo2: public IContactInfo {
public:
    GenericLineContactInfo2(
        RigidBodyPulses& rbp0,
        RigidBodyPulses& rbp1,
        const GenericLineEqualityConstraint<tnullspace>& lec);
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) override;
private:
    RigidBodyPulses& rbp0_;
    RigidBodyPulses& rbp1_;
    GenericLineEqualityConstraint<tnullspace> lec_;
};

using PointContactInfo2 = GenericLineContactInfo2<0>;
using LineContactInfo2 = GenericLineContactInfo2<1>;

class PlaneContactInfo1: public IContactInfo {
public:
    PlaneContactInfo1(
        RigidBodyPulses& rbp0,
        const FixedArray<float, 3>& v1,
        const BoundedPlaneEqualityConstraint& pec);
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) override;
private:
    RigidBodyPulses& rbp0_;
    FixedArray<float, 3> v1_;
    BoundedPlaneEqualityConstraint pec_;
};

class PlaneContactInfo2: public IContactInfo {
public:
    PlaneContactInfo2(
        RigidBodyPulses& rbp0,
        RigidBodyPulses& rbp1,
        const BoundedPlaneEqualityConstraint& pec);
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) override;
private:
    RigidBodyPulses& rbp0_;
    RigidBodyPulses& rbp1_;
    BoundedPlaneEqualityConstraint pec_;
};

template <class TRigidBodyPulsesArg, class TRigidBodyPulsesField>
class GenericNormalContactInfo1: public IContactInfo {
public:
    GenericNormalContactInfo1(
        TRigidBodyPulsesArg rbp,
        const BoundedPlaneInequalityConstraint& pc,
        const FixedArray<ScenePos, 3>& p);
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) override;
    const NormalImpulse& normal_impulse() const {
        return pc_.constraint.normal_impulse;
    }
private:
    TRigidBodyPulsesField rbp_;
    BoundedPlaneInequalityConstraint pc_;
    FixedArray<ScenePos, 3> p_;
};

using NormalContactInfo1 = GenericNormalContactInfo1<RigidBodyPulses&, RigidBodyPulses&>;
using AttachedWheelNormalContactInfo1 = GenericNormalContactInfo1<const AttachedWheel&, AttachedWheel>;

class NormalContactInfo2: public IContactInfo {
public:
    NormalContactInfo2(
        RigidBodyPulses& rbp0,
        RigidBodyPulses& rbp1,
        const BoundedPlaneInequalityConstraint& pc,
        const FixedArray<ScenePos, 3>& p,
        const std::function<void(float)>& notify_lambda_final = [](float){});
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) override;
    virtual void finalize() override;
    const NormalImpulse& normal_impulse() const {
        return pc_.constraint.normal_impulse;
    }
private:
    RigidBodyPulses& rbp0_;
    RigidBodyPulses& rbp1_;
    BoundedPlaneInequalityConstraint pc_;
    FixedArray<ScenePos, 3> p_;
    std::function<void(float)> notify_lambda_final_;
};

class ShockAbsorberContactInfo1: public IContactInfo {
public:
    ShockAbsorberContactInfo1(
        RigidBodyPulses& rbp,
        const BoundedShockAbsorberConstraint& sc,
        const FixedArray<ScenePos, 3>& p);
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) override;
    const NormalImpulse& normal_impulse() const {
        return sc_.constraint.normal_impulse;
    }
private:
    RigidBodyPulses& rbp_;
    BoundedShockAbsorberConstraint sc_;
    FixedArray<ScenePos, 3> p_;
};

class ShockAbsorberContactInfo2: public IContactInfo {
public:
    ShockAbsorberContactInfo2(
        RigidBodyPulses& rbp0,
        RigidBodyPulses& rbp1,
        const BoundedShockAbsorberConstraint& sc,
        const FixedArray<ScenePos, 3>& p);
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) override;
    const NormalImpulse& normal_impulse() const {
        return sc_.constraint.normal_impulse;
    }
private:
    RigidBodyPulses& rbp0_;
    RigidBodyPulses& rbp1_;
    BoundedShockAbsorberConstraint sc_;
    FixedArray<ScenePos, 3> p_;
};

class FrictionContactInfo1: public IContactInfo {
    friend std::ostream& operator << (std::ostream& ostr, const FrictionContactInfo1& fci1);
public:
    FrictionContactInfo1(
        RigidBodyPulses& rbp,
        const NormalImpulse& normal_impulse,
        const FixedArray<ScenePos, 3>& p,
        float stiction_coefficient,
        float friction_coefficient,
        const FixedArray<float, 3>& b,
        const FixedArray<float, 3>& clamping_direction = fixed_nans<float, 3>(),
        float clamping_min = NAN,
        float clamping_max = NAN,
        float ortho_clamping_max_l2 = NAN,
        float extra_stiction = 0,
        float extra_friction = 0,
        float extra_w = 0);
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) override;
    float max_impulse_stiction() const;
    float max_impulse_friction() const;
    const FixedArray<float, 3>& get_b() const;
    void set_b(const FixedArray<float, 3>& b);
    void set_clamping(
        const FixedArray<float, 3>& clamping_direction,
        float clamping_min,
        float clamping_max,
        float ortho_clamping_max_l2_);
    void set_extras(
        float extra_stiction,
        float extra_friction,
        float extra_w);
    const NormalImpulse& normal_impulse() const {
        return normal_impulse_;
    }
    const FixedArray<float, 3>& lambda_total() const {
        return lambda_total_;
    }
private:
    FixedArray<float, 3> lambda_total_;
    FixedArray<float, 3> b_;
    RigidBodyPulses& rbp_;
    const NormalImpulse& normal_impulse_;
    FixedArray<ScenePos, 3> p_;
    float stiction_coefficient_;
    float friction_coefficient_;
    FixedArray<float, 3> clamping_direction_;
    float clamping_min_;
    float clamping_max_;
    float ortho_clamping_max_l2_;
    float extra_stiction_;
    float extra_friction_;
    float extra_w_;
};

std::ostream& operator << (std::ostream& ostr, const FrictionContactInfo1& fci1);

class FrictionContactInfo2: public IContactInfo {
public:
    FrictionContactInfo2(
        RigidBodyPulses& rbp0,
        RigidBodyPulses& rbp1,
        const NormalImpulse& normal_impulse,
        const FixedArray<ScenePos, 3>& p,
        float stiction_coefficient,
        float friction_coefficient,
        const FixedArray<float, 3>& b);
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) override;
    float max_impulse_stiction() const;
    float max_impulse_friction() const;
    void set_b(const FixedArray<float, 3>& b);
private:
    FixedArray<float, 3> lambda_total_;
    FixedArray<float, 3> b_;
    RigidBodyPulses& rbp0_;
    RigidBodyPulses& rbp1_;
    const NormalImpulse& normal_impulse_;
    FixedArray<ScenePos, 3> p_;
    float stiction_coefficient_;
    float friction_coefficient_;
};

class TireContactInfo1: public IContactInfo {
public:
    TireContactInfo1(
        const FrictionContactInfo1& fci,
        float surface_stiction_factor,
        RigidBodyVehicle& rb,
        size_t tire_id,
        const FixedArray<float, 3>& vc_street,
        const FixedArray<float, 3>& vc,
        const FixedArray<float, 3>& n3,
        float v0,
        const PhysicsEngineConfig& cfg);
    virtual void solve(float dt, float relaxation, size_t iteration, size_t niterations) override;
private:
    FrictionContactInfo1 fci_;
    float surface_stiction_factor_;
    RigidBodyVehicle& rb_;
    TirePowerIntent P_;
    size_t tire_id_;
    const FixedArray<float, 3> vc_street_;
    FixedArray<float, 3> vc_;
    FixedArray<float, 3> n3_;
    float v0_;
    FixedArray<float, 3> b0_;
    const PhysicsEngineConfig& cfg_;
};

void solve_contacts(std::list<std::unique_ptr<IContactInfo>>& cis, float dt);

}
