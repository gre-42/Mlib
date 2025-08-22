#pragma once
#include <Mlib/Audio/Lazy_One_Shot_Audio.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Physics/Smoke_Generation/Particle_Descriptor.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Mlib {

struct SurfaceSmokeRule {
    // Has a standard constructor for JSON parsing (needs to be default-constructible).
    Interp<float> smoke_particle_frequency = Interp<float>{ {}, {} };
    Interp<float> smoke_particle_velocity = Interp<float>{ {}, {} };
};

struct SurfaceSmokeVisual {
    ParticleDescriptor particle;
    std::string smoke_particle_instance_prefix;
};

enum class SurfaceSmokeAffinity {
    PAIR,
    TIRE
};

SurfaceSmokeAffinity surface_smoke_affinity_from_string(const std::string& s);

struct SurfaceSmokeInfo {
    std::optional<SurfaceSmokeVisual> visual;
    std::unique_ptr<LazyOneShotAudio> audio;
    std::optional<VariableAndHash<std::string>> audio_resource_name;
    SurfaceSmokeAffinity affinity;
    SurfaceSmokeRule vehicle_velocity;
    SurfaceSmokeRule tire_velocity;
};

struct SurfaceContactInfo {
    float stiction_factor;      // for tires (tires store their stiction coefficient themselves)
    float stiction_coefficient; // for everything except tires
    float friction_coefficient; // for everything except tires
    std::vector<SurfaceSmokeInfo> emission;
};

}
