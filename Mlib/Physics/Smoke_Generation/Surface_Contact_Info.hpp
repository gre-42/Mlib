#pragma once
#include <Mlib/Math/Interp.hpp>
#include <string>

namespace Mlib {

struct SurfaceContactInfo {
    float surface_stiction_factor;
    float minimum_velocity_for_smoke;
    std::string smoke_particle_resource_name;
    std::string smoke_particle_instance_prefix;
    Interp<float> velocity_to_smoke_particle_frequency;
    float smoke_particle_animation_duration;
};

}
