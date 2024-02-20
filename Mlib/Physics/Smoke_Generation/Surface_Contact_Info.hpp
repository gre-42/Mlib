#pragma once
#include <Mlib/Math/Interp.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct SurfaceSmokeInfo {
    std::string smoke_particle_resource_name;
    std::string smoke_particle_instance_prefix;
    // Has a standard constructor for JSON parsing (needs to be default-constructible).
    Interp<float> vehicle_velocity_to_smoke_particle_frequency = Interp<float>{ {}, {} };
    Interp<float> tire_velocity_to_smoke_particle_frequency = Interp<float>{ {}, {} };
    float smoke_particle_animation_duration;
};

struct SurfaceContactInfo {
    float surface_stiction_factor;
    std::vector<SurfaceSmokeInfo> smoke_infos;
};

}
