#pragma once
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Smoke_Generation/Particle_Descriptor.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>
#include <vector>

namespace Mlib {

struct SurfaceSmokeRule {
    // Has a standard constructor for JSON parsing (needs to be default-constructible).
    Interp<float> smoke_particle_frequency = Interp<float>{ {}, {} };
    Interp<float> smoke_particle_velocity = Interp<float>{ {}, {} };
};

struct SurfaceSmokeInfo {
    ParticleDescriptor particle;
    std::string smoke_particle_instance_prefix;
    SurfaceSmokeRule vehicle_velocity;
    SurfaceSmokeRule tire_velocity;
};

struct SurfaceContactInfo {
    float stiction_factor;      // for tires (tires store their stiction coefficient themselves)
    float stiction_coefficient; // for everything except tires
    float friction_coefficient; // for everything except tires
    std::vector<SurfaceSmokeInfo> smoke_infos;
};

}
