#pragma once
#include <string>

namespace Mlib {

struct SurfaceContactInfo {
    float surface_stiction_factor;
    float minimum_velocity_for_smoke;
    std::string smoke_particle_resource_name;
    std::string smoke_particle_instance_prefix;
    float smoke_particle_generation_dt;
    float smoke_particle_animation_duration;
};

}
