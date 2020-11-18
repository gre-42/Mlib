#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Engine.hpp>
#include <list>

namespace Mlib {

class RigidBody;
struct RigidBodyPulses;
struct PhysicsEngineConfig;

struct VelocityConstraint {
    FixedArray<float, 3> normal;
    float b;
    float lambda_total = 0;
    inline float C(const FixedArray<float, 3>& x) const {
        return -dot0d(normal, x);
    }
    inline float v() const {
        return b;
    }
};

struct PlaneConstraint {
    PlaneNd<float, 3> plane;
    float b = 0;
    float slop = 0;
    float lambda_total = 0;
    bool always_active = true;
    float beta = 0.5;
    float beta2 = 0.2;
    inline float C(const FixedArray<float, 3>& x) const {
        return -(dot0d(plane.normal_, x) + plane.intercept_);
    }
    inline float overlap(const FixedArray<float, 3>& x) const {
        // std::cerr << plane.normal_ << " | " << x << " | " << plane.intercept_ << std::endl;
        return -(dot0d(plane.normal_, x) + plane.intercept_);
    }
    inline float active(const FixedArray<float, 3>& x) const {
        return always_active || (overlap(x) > 0);
    }
    inline float bias(const FixedArray<float, 3>& x) const {
        return std::max(0.f, overlap(x) - slop);
    }
    inline float v(const FixedArray<float, 3>& p, float dt) const {
        return b + 1.f / dt * (beta * C(p) - beta2 * bias(p));
    }
};

struct BoundedPlaneConstraint {
    PlaneConstraint plane_constraint;
    float lambda_min = -INFINITY;
    float lambda_max = INFINITY;
    inline float clamped_lambda(float lambda) {
        lambda = std::clamp(plane_constraint.lambda_total + lambda, lambda_min, lambda_max) - plane_constraint.lambda_total;
        plane_constraint.lambda_total += lambda;
        return lambda;
    }
};

class ContactInfo {
public:
    virtual void solve(float dt, float relaxation) = 0;
    virtual ~ContactInfo() = default;
};

class ContactInfo1: public ContactInfo {
public:
    ContactInfo1(
        RigidBodyPulses& rbp,
        const BoundedPlaneConstraint& pc,
        const FixedArray<float, 3>& p);
    void solve(float dt, float relaxation) override;
    const PlaneConstraint& pc() const {
        return pc_.plane_constraint;
    }
private:
    RigidBodyPulses& rbp_;
    BoundedPlaneConstraint pc_;
    FixedArray<float, 3> p_;
};

class ContactInfo2: public ContactInfo {
public:
    ContactInfo2(
        RigidBodyPulses& rbp0,
        RigidBodyPulses& rbp1,
        const BoundedPlaneConstraint& pc,
        const FixedArray<float, 3>& p);
    void solve(float dt, float relaxation) override;
    const PlaneConstraint& pc() const {
        return pc_.plane_constraint;
    }
private:
    RigidBodyPulses& rbp0_;
    RigidBodyPulses& rbp1_;
    BoundedPlaneConstraint pc_;
    FixedArray<float, 3> p_;
};

class OrthoVelocityConstraints {
public:
    explicit OrthoVelocityConstraints(
        const FixedArray<float, 3>& normal,
        const FixedArray<float, 3>& b);
    void set_b(const FixedArray<float, 3>& b);
protected:
    VelocityConstraint pcs_[2];
};

class FrictionContactInfo1: public OrthoVelocityConstraints, public ContactInfo {
public:
    FrictionContactInfo1(
        RigidBodyPulses& rbp,
        const PlaneConstraint& normal_constraint,
        const FixedArray<float, 3>& p,
        float stiction_coefficient,
        float friction_coefficient,
        const FixedArray<float, 3>& b);
    void solve(float dt, float relaxation) override;
    float max_impulse() const;
private:
    RigidBodyPulses& rbp_;
    const PlaneConstraint& normal_constraint_;
    FixedArray<float, 3> p_;
    float stiction_coefficient_;
    float friction_coefficient_;
};

class FrictionContactInfo2: public OrthoVelocityConstraints, public ContactInfo {
public:
    FrictionContactInfo2(
        RigidBodyPulses& rbp0,
        RigidBodyPulses& rbp1,
        const PlaneConstraint& normal_constraint,
        const FixedArray<float, 3>& p,
        float stiction_coefficient,
        float friction_coefficient,
        const FixedArray<float, 3>& b);
    void solve(float dt, float relaxation) override;
    float max_impulse() const;
private:
    RigidBodyPulses& rbp0_;
    RigidBodyPulses& rbp1_;
    const PlaneConstraint& normal_constraint_;
    FixedArray<float, 3> p_;
    float stiction_coefficient_;
    float friction_coefficient_;
};

class TireContactInfo1: public ContactInfo {
public:
    TireContactInfo1(
        const FrictionContactInfo1& fci,
        RigidBody& rb,
        size_t tire_id,
        const FixedArray<float, 3>& v3,
        const FixedArray<float, 3>& n3,
        const PhysicsEngineConfig& cfg);
    void solve(float dt, float relaxation) override;
private:
    FrictionContactInfo1 fci_;
    RigidBody& rb_;
    PowerIntent P_;
    size_t tire_id_;
    FixedArray<float, 3> v3_;
    FixedArray<float, 3> n3_;
    const PhysicsEngineConfig& cfg_;
};

void solve_contacts(std::list<std::unique_ptr<ContactInfo>>& cis, float dt);

}
