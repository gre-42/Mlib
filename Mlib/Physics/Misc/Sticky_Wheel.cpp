#include "Sticky_Wheel.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>

using namespace Mlib;

StickyWheel::StickyWheel(
    const FixedArray<float, 3>& rotation_axis,
    size_t nsprings,
    float max_dist)
: rotation_axis_{rotation_axis},
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
    s.active = true;
    s.position = dot1d(rotation.T(), pt_absolute - translation);
    s.normal = normal;
    s.spring.point_of_contact = pt_absolute;
    next_spring_ = (next_spring_ + 1) % springs_.size();
}

void StickyWheel::update_position(
    const FixedArray<float, 3, 3>& rotation,
    const FixedArray<float, 3>& translation,
    const FixedArray<float, 3>& power_axis,
    float spring_constant,
    float stiction_force,
    float dt,
    FixedArray<float, 3>& force,
    float& power,
    std::vector<FixedArray<float, 3>>& beacons)
{
    angle_x_ += w_ * dt;
    angle_x_ = std::fmod(angle_x_, 2 * M_PI);
    FixedArray<float, 3, 3> dr = rodrigues(rotation_axis_, w_ * dt);
    force = 0;
    power = 0;
    for(auto& s : springs_) {
        if (s.active) {
            // std::cerr << "v " << std::sqrt(sum(squared(s.position))) * w_ * 3.6 << std::endl;
            FixedArray<float, 3> abs_position = dot1d(rotation, s.position) + translation;
            // std::cerr << "d " << abs_position << " | " << s.spring.point_of_contact << " | " << (abs_position - s.spring.point_of_contact) << std::endl;
            if (sum(squared(abs_position - s.spring.point_of_contact)) > squared(max_dist_)) {
                s.active = false;
            } else {
                beacons.push_back(abs_position);
                // beacons.push_back(s.spring.point_of_contact);
                FixedArray<float, 3> f = s.spring.update_position(
                    abs_position,
                    spring_constant / springs_.size(),
                    stiction_force / springs_.size(),
                    &s.normal);
                force += f;
                power += dot0d(power_axis, f) * std::sqrt(sum(squared(s.position))) * w_;
                s.position = dot1d(dr, s.position);
            }
        }
    }
    // std::cerr << 0.00135962 * power << " PS " << " F " << std::sqrt(sum(squared(force))) << std::endl;
}

void StickyWheel::accelerate(float amount) {
    w_ += amount;
}

float StickyWheel::angle_x() const {
    return angle_x_;
}
