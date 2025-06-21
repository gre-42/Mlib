#pragma once
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

ParticleType material_skidmarks(PhysicsMaterial material);

}
