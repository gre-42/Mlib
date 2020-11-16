#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Plane_Nd.hpp>
#include <list>

namespace Mlib {

struct RigidBodyPulses;

struct PlaneConstraint {
    PlaneNd<float, 3> plane;
    float b;
    float slop;
    float lambda_min = -INFINITY;
    float lambda_max = INFINITY;
    inline float C(const FixedArray<float, 3>& x) const {
        return -(dot0d(plane.normal_, x) + plane.intercept_);
    }
    inline float overlap(const FixedArray<float, 3>& x) const {
        // std::cerr << plane.normal_ << " | " << x << " | " << plane.intercept_ << std::endl;
        return -(dot0d(plane.normal_, x) + plane.intercept_);
    }
    inline float active(const FixedArray<float, 3>& x) const {
        return overlap(x) > 0;
    }
    inline float bias(const FixedArray<float, 3>& x) const {
        return std::max(0.f, overlap(x) - slop);
    }
};

struct ContactPoint {
    float beta;
    float beta2;
    FixedArray<float, 3> p;
    inline float v(const PlaneConstraint& pc, float dt) const {
        return pc.b + 1.f / dt * (beta * pc.C(p) - beta2 * pc.bias(p));
    }
};

class ContactInfo {
public:
    virtual void solve(float dt, float* lambda_total = nullptr) const = 0;
    virtual ~ContactInfo() = default;
};

class ContactInfo1: public ContactInfo {
public:
    ContactInfo1(
        RigidBodyPulses& rbp,
        const PlaneConstraint& pc,
        const ContactPoint& cp)
    : rbp{rbp},
      pc{pc},
      cp{cp}
    {}
    void solve(float dt, float* lambda_total = nullptr) const override;
private:
    RigidBodyPulses& rbp;
    PlaneConstraint pc;
    ContactPoint cp;
};

class ContactInfo2: public ContactInfo {
public:
    ContactInfo2(
        RigidBodyPulses& rbp0,
        RigidBodyPulses& rbp1,
        const PlaneConstraint& pc,
        const ContactPoint& cp)
    : rbp0{rbp0},
      rbp1{rbp1},
      pc{pc},
      cp{cp}
    {}
    void solve(float dt, float* lambda_total = nullptr) const override;
private:
    RigidBodyPulses& rbp0;
    RigidBodyPulses& rbp1;
    PlaneConstraint pc;
    ContactPoint cp;
};

void solve_contacts(std::list<std::unique_ptr<ContactInfo>>& cis, float dt);

}
