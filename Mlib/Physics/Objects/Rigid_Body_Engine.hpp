#pragma once
#include <cstddef>
#include <vector>

namespace Mlib {

class RigidBodyEngine {
public:
    explicit RigidBodyEngine(float max_surface_power);
    void set_surface_power(float surface_power);
    float consume_abs_surface_power(size_t ntires);
    void reset_forces();

private:
    float surface_power_;
    size_t surface_power_nconsumed_;
    float max_surface_power_;
};

}
