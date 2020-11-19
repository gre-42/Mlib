#include "Tracking_Wheel.hpp"
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Integrator.hpp>

using namespace Mlib;

TrackingWheel::TrackingWheel(
    const FixedArray<float, 3>& rotation_axis,
    float radius,
    size_t nsprings,
    float max_dist,
    float dt)
: rotation_axis_{rotation_axis},
  radius_{radius},
  springs_(nsprings, TrackingSpring{.active = false, .spring = {.pid{1, 0, float(1e-1) / dt, 0}}}),
  max_dist_{max_dist},
  next_spring_{0},
  w_{0},
  angle_x_{0},
  sum_stiction_force_{0},
  sum_friction_force_{0},
  pose_initialized_{false}
{}

void TrackingWheel::notify_intersection(
    const FixedArray<float, 3, 3>& rotation,
    const FixedArray<float, 3>& translation,
    const FixedArray<float, 3>& pt_absolute,
    const FixedArray<float, 3>& normal,
    float stiction_force,
    float friction_force)
{
    sum_stiction_force_ += stiction_force;
    sum_friction_force_ += friction_force;
    for(TrackingSpring& s : springs_) {
        if (s.active) {
            if (sum(squared(s.position - s.spring.point_of_contact)) > squared(max_dist_)) {
                s.active = false;
            } else if (
                FixedArray<float, 3> abs_position = dot1d(rotation, s.position) + translation;
                sum(squared(abs_position - pt_absolute)) < squared(max_dist_))
            {
                s.found = true;
                // s.position = FixedArray<float, 3>{0, -radius_, 0};
                s.position = dot1d(rotation.T(), pt_absolute - translation);
                s.normal = dot1d(rotation.T(), normal);
                s.spring.point_of_contact += s.normal * dot0d(s.normal, s.position - s.spring.point_of_contact);
                // std::cerr << s.position << " | " << s.spring.point_of_contact << std::endl;
                return;
            }
        }
    }
    auto sit = std::find_if(springs_.begin(), springs_.end(), [](TrackingSpring& s){return !s.active;});
    if (sit != springs_.end()) {
        sit->active = true;
        sit->found = true;
        // sit->position = FixedArray<float, 3>{0, -radius_, 0};
        sit->position = dot1d(rotation.T(), pt_absolute - translation);
        sit->normal = dot1d(rotation.T(), normal);
        sit->spring.point_of_contact = sit->position;
    }
}

void TrackingWheel::update_position(
    const FixedArray<float, 3, 3>& rotation,
    const FixedArray<float, 3>& translation,
    const FixedArray<float, 3>& power_axis,
    const FixedArray<float, 3>& velocity,
    float spring_constant,
    float dt,
    RigidBodyIntegrator& rbi,
    float& power_internal,
    float& power_external,
    float& moment,
    bool& slipping,
    std::list<Beacon>& beacons)
{
    if (!pose_initialized_) {
        old_rotation_ = rotation;
        old_translation_ = translation;
        pose_initialized_ = true;
    }
    angle_x_ += w_ * dt;
    angle_x_ = std::fmod(angle_x_, 2 * M_PI);
    power_internal = 0;
    power_external = 0;
    moment = 0;
    size_t nactive = 0;
    for(auto& s : springs_) {
        if (s.active) {
            if (!s.found) {
                s.active = false;
            } else {
                ++nactive;
            }
        }
    }
    size_t nslipping = 0;
    for(auto& s : springs_) {
        if (s.active) {
            // std::cerr << "old_rotation_" << std::endl;
            // std::cerr << old_rotation_ << std::endl;
            // std::cerr << "old_translation_" << std::endl;
            // std::cerr << old_translation_ << std::endl;
            // std::cerr << "rotation" << std::endl;
            // std::cerr << rotation << std::endl;
            // std::cerr << "translation" << std::endl;
            // std::cerr << translation << std::endl;
            // std::cerr << "old " << s.spring.point_of_contact << std::endl;
            {
                FixedArray<float, 3> dir = dot1d(rotation.T(), old_translation_ - translation);
                dir -= s.normal * dot0d(dir, s.normal);
                s.spring.point_of_contact += dir;
            }
            // std::cerr << "new " << s.spring.point_of_contact << std::endl;
            // std::cerr << "normal " << s.normal << std::endl;
            FixedArray<float, 3> force;
            bool slip;
            // std::cerr << "old2 " << s.spring.point_of_contact << " | " << s.normal << " | " << s.position << std::endl;
            s.spring.update_position(
                s.position,
                spring_constant / springs_.size(),
                sum_stiction_force_ / nactive,
                sum_friction_force_ / nactive,
                &s.normal,
                true,  // move_point_of_contact
                force,
                slip);
            // std::cerr << "new2 " << s.spring.point_of_contact << " | " << s.normal << std::endl;
            // assert_true(s.spring.point_of_contact(1) < 0);
            nslipping += slip;
            FixedArray<float, 3> abs_pos = dot1d(rotation, s.position) + translation;
            FixedArray<float, 3> abs_force = dot1d(rotation, force);
            rbi.integrate_force({.vector = abs_force, .position = abs_pos});
            // W = F * s
            // dW/dt = F * ds/dt
            // P = F * v
            float cmoment = dot0d(abs_force, power_axis) * std::sqrt(sum(squared(s.position)));
            moment += cmoment;
            power_internal += cmoment * w_;
            power_external -= dot0d(abs_force, velocity);
            // std::cerr << "abs_poc1 " << dot1d(rotation, s.spring.point_of_contact) << std::endl;
            // std::cerr << "abs_poc2 " << dot1d(rotation, s.spring.point_of_contact - FixedArray<float, 3>{0, 0, 1} * w_ * radius_ * dt * 1000.f) << std::endl;
            {
                auto np = dot1d(rotation.T(), power_axis);
                np -= s.normal * dot0d(s.normal, np);
                if (float l2 = sum(squared(np)); l2 > 1e-9) {
                    np /= std::sqrt(l2);
                    s.spring.point_of_contact -= np * w_ * radius_ * dt;
                    // std::cerr << "np " << np << std::endl;
                }
            }
            // std::cerr << "new3 " << s.spring.point_of_contact << " | " << s.normal << std::endl;
            if (slip) {
                beacons.push_back({dot1d(rotation, s.spring.point_of_contact) + translation});
                beacons.push_back({position: abs_pos, resource_name: "beacon1"});
            }
        }
    }
    slipping = (nslipping > nactive / 2);
    sum_stiction_force_ = 0;
    sum_friction_force_ = 0;
    for(auto& s : springs_) {
        s.found = false;
    }
    old_rotation_ = rotation;
    old_translation_ = translation;
}

float TrackingWheel::radius() const {
    return radius_;
}

float TrackingWheel::w() const {
    return w_;
}

void TrackingWheel::set_w(float w) {
    w_ = w;
}

float TrackingWheel::angle_x() const {
    return angle_x_;
}
