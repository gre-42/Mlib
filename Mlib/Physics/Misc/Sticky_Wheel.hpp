#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Misc/Sticky_Spring.hpp>


namespace Mlib {

class StickyWheel {
public:
    explicit StickyWheel(const FixedArray<float, 3>& rotation_axis, float radius, size_t nsprings, float max_dist)
    : rotation_axis_{rotation_axis},
      radius_{radius},
      springs_(nsprings, {FixedArray<float, 3>(NAN), StickySpring{}}),
      max_dist_{max_dist},
      next_spring_{0},
      w_{0}
    {}
    void notify_intersection(
        const FixedArray<float, 3, 3>& rotation,
        const FixedArray<float, 3>& translation,
        const FixedArray<float, 3>& pt_absolute)
    {
        auto& s = springs_[next_spring_];
        s.first = dot1d(rotation.T(), pt_absolute - translation);
        s.second.point_of_contact = pt_absolute;
        next_spring_ = (next_spring_ + 1) % springs_.size();
    }
    FixedArray<float, 3> update_position(
        const FixedArray<float, 3, 3>& rotation,
        const FixedArray<float, 3>& translation,
        float spring_constant,
        float stiction_force,
        float dt)
    {
        FixedArray<float, 3, 3> dr = rodrigues(rotation_axis_, w_ * dt);
        FixedArray<float, 3> f(0);
        for(auto& s : springs_) {
            if (!any(isnan(s.first))) {
                s.first = dot1d(dr, s.first);
                f += s.second.update_position(dot1d(rotation, s.first) + translation, spring_constant, stiction_force);
            }
        }
        return f;
    }
    void accelerate(float amount) {
        w_ += amount;
    }
private:
    FixedArray<float, 3> rotation_axis_;
    float radius_;
    std::vector<std::pair<FixedArray<float, 3>, StickySpring>> springs_;
    float max_dist_;
    size_t next_spring_;
    float w_;    // angular velocity
};

}
