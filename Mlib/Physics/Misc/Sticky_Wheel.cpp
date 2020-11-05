#include "Sticky_Wheel.hpp"
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Integrator.hpp>

using namespace Mlib;

StickyWheel::StickyWheel(
    const FixedArray<float, 3>& rotation_axis,
    float radius,
    size_t nsprings,
    float max_dist)
: rotation_axis_{rotation_axis},
  radius_{radius},
  springs_(nsprings, SpringExt{.active = false}),
  max_dist_{max_dist},
  next_spring_{0},
  w_{0},
  angle_x_{0},
  sum_stiction_force_{0},
  sum_friction_force_{0}
{
    if (springs_.size() < 2) {
        throw std::runtime_error("Need at least 2 springs");
    }
}

void StickyWheel::notify_intersection(
    const FixedArray<float, 3, 3>& rotation,
    const FixedArray<float, 3>& translation,
    const FixedArray<float, 3>& pt_absolute,
    const FixedArray<float, 3>& normal,
    float stiction_force,
    float friction_force)
{
    sum_stiction_force_ += stiction_force;
    sum_friction_force_ += friction_force;
    SpringExt* ps;
    auto sit = std::find_if(springs_.begin(), springs_.end(), [](SpringExt& s){return !s.active;});
    if (sit == springs_.end()) {
        for(auto& s : springs_) {
            FixedArray<float, 3> abs_position = dot1d(rotation, s.position) + translation;
            if (sum(squared(abs_position - pt_absolute)) < squared(0.02)) {
                return;
            }
        }
        ps = &springs_[next_spring_];
        next_spring_ = (next_spring_ + 1) % springs_.size();
    } else {
        ps = &*sit;
    }
    ps->active = true;
    ps->position = dot1d(rotation.T(), pt_absolute - translation);
    ps->normal = normal;
    ps->spring.point_of_contact = pt_absolute;
}

void StickyWheel::update_position(
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
    std::list<FixedArray<float, 3>>& beacons)
{
    angle_x_ += w_ * dt;
    angle_x_ = std::fmod(angle_x_, 2 * M_PI);
    FixedArray<float, 3, 3> dr = rodrigues(rotation_axis_, w_ * dt);
    power_internal = 0;
    power_external = 0;
    moment = 0;
    size_t nactive = 0;
    for(auto& s : springs_) {
        if (s.active) {
            FixedArray<float, 3> abs_position = dot1d(rotation, s.position) + translation;
            if (sum(squared(abs_position - s.spring.point_of_contact)) > squared(max_dist_)) {
                // std::cerr << abs_position << " | " << s.spring.point_of_contact << std::endl;
                s.active = false;
            } else {
                ++nactive;
            }
        }
    }
    size_t nslipping = 0;
    for(auto& s : springs_) {
        if (s.active) {
            // std::cerr << "v " << std::sqrt(sum(squared(s.position))) * w_ * 3.6 << std::endl;
            FixedArray<float, 3> abs_position = dot1d(rotation, s.position) + translation;
            // std::cerr << "d " << abs_position << " | " << s.spring.point_of_contact << " | " << (abs_position - s.spring.point_of_contact) << std::endl;
            // beacons.push_back(abs_position);
            // beacons.push_back(s.spring.point_of_contact);
            FixedArray<float, 3> force;
            bool slip;
            s.spring.update_position(
                abs_position,
                spring_constant / springs_.size(),
                sum_stiction_force_ / nactive,
                sum_friction_force_ / nactive,
                &s.normal,
                force,
                slip);
            nslipping += slip;
            rbi.integrate_force({vector: force, position: s.spring.point_of_contact});
            // W = F * s
            // dW/dt = F * ds/dt
            // P = F * v
            float cmoment = dot0d(force, power_axis) * std::sqrt(sum(squared(s.position)));
            moment += cmoment;
            power_internal += cmoment * w_;
            power_external -= dot0d(force, velocity);
            s.position = dot1d(dr, s.position);
            // s.position -= FixedArray<float, 3>{0, 0, 1} * w_ * radius_ * dt;
            if (slip) {
                beacons.push_back(abs_position);
                beacons.push_back(s.spring.point_of_contact);
            }
        }
    }
    // std::cerr << nslipping << " " << nactive << std::endl;
    slipping = (nslipping >= nactive / 2);
    if (slipping) {
        beacons.push_back(translation);
        if (false) {
            static size_t ct = 0;
            ++ct;
            std::cerr << "ct " << ct << std::endl;
            if (ct > 14000) {
                std::ofstream svg_file{"/tmp/springs.svg"};
                Svg svg{svg_file, 500, 500};
                std::vector<float> x;
                std::vector<float> y;
                for(auto& s : springs_) {
                    if (s.active) {
                        FixedArray<float, 3> abs_position = dot1d(rotation, s.position) + translation;
                        x.push_back(abs_position(0));
                        y.push_back(abs_position(2));
                    }
                }
                svg.plot(x, y);
                svg.finish();
            }
            if (ct > 14000) {
                std::ofstream svg_file{"/tmp/springs2.svg"};
                Svg svg{svg_file, 500, 500};
                std::vector<float> x;
                std::vector<float> y;
                for(auto& s : springs_) {
                    if (s.active) {
                        x.push_back(s.spring.point_of_contact(0));
                        y.push_back(s.spring.point_of_contact(2));
                    }
                }
                svg.plot(x, y);
                svg.finish();
            }
        }
    }
    // std::cerr << nslipping << " " << int(slipping) << std::endl;
    // std::cerr << "nactive " << nactive << std::endl;
    sum_stiction_force_ = 0;
    sum_friction_force_ = 0;
    // std::cerr << 0.00135962 * power << " PS " << " F " << std::sqrt(sum(squared(force))) << std::endl;
}

float StickyWheel::radius() const {
    return radius_;
}

float StickyWheel::w() const {
    return w_;
}

void StickyWheel::set_w(float w) {
    w_ = w;
}

float StickyWheel::angle_x() const {
    return angle_x_;
}
