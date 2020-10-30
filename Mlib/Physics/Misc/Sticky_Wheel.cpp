#include "Sticky_Wheel.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>

using namespace Mlib;

StickyWheel::StickyWheel(
    const FixedArray<float, 3>& rotation_axis,
    float radius,
    size_t nsprings,
    float max_dist)
: rotation_axis_{rotation_axis},
    radius_{radius},
    springs_(nsprings, SpringExt{position: FixedArray<float, 3>(NAN), spring: StickySpring{}}),
    max_dist_{max_dist},
    next_spring_{0},
    w_{0},
    angle_x_{0}
{}

void StickyWheel::notify_intersection(
    const FixedArray<float, 3, 3>& rotation,
    const FixedArray<float, 3>& translation,
    const FixedArray<float, 3>& pt_absolute,
    const FixedArray<float, 3>& normal)
{
    auto& s = springs_[next_spring_];
    s.position = dot1d(rotation.T(), pt_absolute - translation);
    s.normal = normal;
    s.spring.point_of_contact = pt_absolute;
    next_spring_ = (next_spring_ + 1) % springs_.size();
}

FixedArray<float, 3> StickyWheel::update_position(
    const FixedArray<float, 3, 3>& rotation,
    const FixedArray<float, 3>& translation,
    float spring_constant,
    float stiction_force,
    float dt,
    std::vector<FixedArray<float, 3>>& beacons)
{
    angle_x_ += w_ * dt;
    angle_x_ = std::fmod(angle_x_, 2 * M_PI);
    FixedArray<float, 3, 3> dr = rodrigues(rotation_axis_, w_ * dt);
    FixedArray<float, 3> f(0);
    for(auto& s : springs_) {
        if (!any(isnan(s.position))) {
            s.position = dot1d(dr, s.position);
            FixedArray<float, 3> abs_position = dot1d(rotation, s.position) + translation;
            // std::cerr << "d " << sum(squared(abs_position - s.spring.point_of_contact)) << std::endl;
            if (sum(squared(abs_position - s.spring.point_of_contact)) > squared(max_dist_)) {
                s.position = NAN;
            } else {
                beacons.push_back(abs_position);
                FixedArray<float, 3> ff = s.spring.update_position(
                    abs_position,
                    spring_constant / springs_.size(),
                    stiction_force / springs_.size());
                f += ff - s.normal * dot0d(ff, s.normal);
            }
        }
    }
    return f;
}

void StickyWheel::accelerate(float amount) {
    w_ += amount;
}

float StickyWheel::angle_x() const {
    return angle_x_;
}
