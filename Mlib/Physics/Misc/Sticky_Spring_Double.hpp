#include "Sticky_Spring.hpp"
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

void update_position_(
    const FixedArray<double, 3>& position,
    double spring_constant,
    double stiction_force,
    double friction_force,
    const FixedArray<double, 3>* normal,
    FixedArray<double, 3>& force,
    bool& slipping,
    FixedArray<double, 3>& point_of_contact)
{
    FixedArray<double, 3> dir = point_of_contact - position;
    if (normal != nullptr) {
        dir -= (*normal) * dot0d(dir, *normal);
    }
    // std::cerr << position << " | " << point_of_contact << " | " << (point_of_contact - position) << std::endl;
    if (double d2 = sum(squared(dir)); d2 > squared(stiction_force / spring_constant)) {
        slipping = true;
        // stiction_force = ||p-c||*k
        // => ||p-c|| = stiction_force/k
        FixedArray<double, 3> d = dir / std::sqrt(d2);
        if (normal != nullptr) {
            point_of_contact -= dir + 0.95 * d * (stiction_force / spring_constant);
        } else {
            point_of_contact = position - 0.95 * d * (stiction_force / spring_constant);
        }
        // auto vv = point_of_contact - position;
        // vv -= (*normal) * dot0d(vv, *normal);
        // std::cerr << "-- " << std::sqrt(sum(squared(vv))) << " " << stiction_force / spring_constant << " | " << d * friction_force << std::endl;
        // if (stiction_force > 1e-4) {
        //     if (sum(squared(vv)) > squared(stiction_force / spring_constant)) {
        //         std::cerr << "-----------" << std::endl;
        //     }
        // }
        // std::cerr << "-- " <<
        //     std::sqrt(sum(squared(vv))) * spring_constant << " " <<
        //     stiction_force << " | " <<
        //     std::sqrt(sum(squared(new_point_of_contact - position))) * spring_constant << " | " <<
        //     stiction_force << " | " <<
        //     d2 << std::endl;
        // std::cerr << "y " << std::sqrt(sum(squared(d * stiction_force))) << " | " << d * stiction_force << std::endl;
        force = d * friction_force;
    } else {
        slipping = false;
        // std::cerr << "x " << std::sqrt(sum(squared(dir * spring_constant))) << " | " << dir * spring_constant << std::endl;
        force = dir * spring_constant;
    }
}

void StickySpring::update_position(
    const FixedArray<float, 3>& position,
    float spring_constant,
    float stiction_force,
    float friction_force,
    const FixedArray<float, 3>* normal,
    FixedArray<float, 3>& force,
    bool& slipping)
{
    FixedArray<double, 3> force_;
    FixedArray<double, 3> point_of_contact_ = point_of_contact.casted<double>();
    FixedArray<double, 3> normal_;
    normal_ = normal->casted<double>();
    update_position_(
        position.casted<double>(),
        spring_constant,
        stiction_force,
        friction_force,
        &normal_,
        force_,
        slipping,
        point_of_contact_);
    force = force_.casted<float>();
    point_of_contact = point_of_contact_.casted<float>();
}
