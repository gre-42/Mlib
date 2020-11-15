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
    float C(const FixedArray<float, 3>& x) const {
        return -(dot0d(plane.normal_, x) + plane.intercept_);
    }
    float overlap(const FixedArray<float, 3>& x) const {
        // std::cerr << plane.normal_ << " | " << x << " | " << plane.intercept_ << std::endl;
        return -(dot0d(plane.normal_, x) + plane.intercept_);
    }
    float active(const FixedArray<float, 3>& x) const {
        return overlap(x) > 0;
    }
    float bias(const FixedArray<float, 3>& x) const {
        return std::max(0.f, overlap(x) - slop);
    }
};

struct ContactInfo {
    RigidBodyPulses& rbp;
    PlaneConstraint pc;
    FixedArray<float, 3> p;
    void solve(float dt, float beta, float beta2);
};

void solve_contacts(std::list<ContactInfo>& cis, float dt, float beta, float beta2);

}
