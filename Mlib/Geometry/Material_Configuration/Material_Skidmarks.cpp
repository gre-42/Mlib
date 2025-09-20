#include "Material_Skidmarks.hpp"
#include <Mlib/Geometry/Material/Particle_Type.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

ParticleType Mlib::material_skidmarks(PhysicsMaterial material) {
    static const std::map<PhysicsMaterial, ParticleType> m{
        {PhysicsMaterial::NONE, ParticleType::NONE},
        {PhysicsMaterial::SURFACE_BASE_TARMAC, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_GRAVEL, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_SNOW, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_ICE, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_SAND, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_GRASS, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_DIRT, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_STONE, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_FOLIAGE, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_GLASS, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_WATER, ParticleType::WATER_WAVE | ParticleType::SEA_SPRAY},
        {PhysicsMaterial::SURFACE_BASE_TARMAC | PhysicsMaterial::SURFACE_WET, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_DIRT | PhysicsMaterial::SURFACE_WET, ParticleType::SKIDMARK},
        {PhysicsMaterial::SURFACE_BASE_DESERT_ROAD, ParticleType::SKIDMARK},
    };
    auto it = m.find(material);
    if (it == m.end()) {
        THROW_OR_ABORT("Cannot find skidmark options for material \"" + physics_material_to_string(material));
    }
    return it->second;
}
