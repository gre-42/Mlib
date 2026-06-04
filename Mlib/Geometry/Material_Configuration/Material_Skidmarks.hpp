#pragma once
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;
enum class ParticleType: uint32_t;

ParticleType material_skidmarks(PhysicsMaterial material);

}
