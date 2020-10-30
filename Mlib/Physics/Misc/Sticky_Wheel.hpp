#pragma once
#include <Mlib/Physics/Misc/Sticky_Spring.hpp>


namespace Mlib {

struct SpringExt {
    FixedArray<float, 3> position;
    FixedArray<float, 3> normal;
    StickySpring spring;
};

class StickyWheel {
public:
    explicit StickyWheel(
        const FixedArray<float, 3>& rotation_axis,
        float radius,
        size_t nsprings,
        float max_dist);
    void notify_intersection(
        const FixedArray<float, 3, 3>& rotation,
        const FixedArray<float, 3>& translation,
        const FixedArray<float, 3>& pt_absolute,
        const FixedArray<float, 3>& normal);
    FixedArray<float, 3> update_position(
        const FixedArray<float, 3, 3>& rotation,
        const FixedArray<float, 3>& translation,
        float spring_constant,
        float stiction_force,
        float dt);
    void accelerate(float amount);
    float angle_x() const;
private:
    FixedArray<float, 3> rotation_axis_;
    float radius_;
    std::vector<SpringExt> springs_;
    float max_dist_;
    size_t next_spring_;
    float w_;    // angular velocity
    float angle_x_;
};

}
