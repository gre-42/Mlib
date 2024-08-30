#pragma once
#include <Mlib/Math/Interp.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct SurfaceSmokeRule {
    // Has a standard constructor for JSON parsing (needs to be default-constructible).
    Interp<float> smoke_particle_frequency = Interp<float>{ {}, {} };
    Interp<float> smoke_particle_velocity = Interp<float>{ {}, {} };
};

struct SurfaceSmokeInfo {
    std::string smoke_particle_resource_name;
    std::string smoke_particle_instance_prefix;
    SurfaceSmokeRule vehicle_velocity;
    SurfaceSmokeRule tire_velocity;
    float air_resistance;
    float smoke_particle_animation_duration;
};

struct SurfaceContactInfo {
    float surface_stiction_factor;
    std::vector<SurfaceSmokeInfo> smoke_infos;
};

}
