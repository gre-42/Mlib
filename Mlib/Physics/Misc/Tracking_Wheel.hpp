#pragma once
#include <Mlib/Physics/Misc/Sticky_Spring.hpp>


namespace Mlib {

struct TrackingSpring {
    bool active;
    bool found;
    FixedArray<float, 3> position;
    FixedArray<float, 3> normal;
    StickySpring spring;
};

struct RigidBodyIntegrator;
struct Beacon;

class TrackingWheel {
public:
    explicit TrackingWheel(
        const FixedArray<float, 3>& rotation_axis,
        float radius,
        size_t nsprings,
        float max_dist,
        float dt);
    void notify_intersection(
        const FixedArray<float, 3, 3>& rotation,
        const FixedArray<float, 3>& translation,
        const FixedArray<float, 3>& pt_absolute,
        const FixedArray<float, 3>& normal,
        float stiction_force,
        float friction_force);
    void update_position(
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
        std::list<Beacon>& beacons);
    float radius() const;
    float w() const;
    void set_w(float w);
    float angle_x() const;
private:
    FixedArray<float, 3> rotation_axis_;
    float radius_;
    std::vector<TrackingSpring> springs_;
    float max_dist_;
    size_t next_spring_;
    float w_;    // angular velocity
    float angle_x_;
    float sum_stiction_force_;
    float sum_friction_force_;
    bool pose_initialized_;
    FixedArray<float, 3, 3> old_rotation_;
    FixedArray<float, 3> old_translation_;
};

}
