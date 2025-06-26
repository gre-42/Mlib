#include "Meta_Materials.hpp"
#include <Mlib/Geometry/Material_Configuration/Base_Materials.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

PhysicsMaterial Mlib::meta_material(PhysicsMaterial material) {
    static const std::map<PhysicsMaterial, PhysicsMaterial> m{
        {PhysicsMaterial::NONE, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_TARMAC, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_GRAVEL, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_SNOW, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_ICE, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_SAND, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_GRASS, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_DIRT, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_STONE, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_FOLIAGE, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_GLASS, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_WATER, META_WATER_MATERIAL},
        {PhysicsMaterial::SURFACE_BASE_TARMAC | PhysicsMaterial::SURFACE_WET, PhysicsMaterial::NONE},
        {PhysicsMaterial::SURFACE_BASE_DIRT | PhysicsMaterial::SURFACE_WET, PhysicsMaterial::NONE},
    };
    auto it = m.find(material);
    if (it == m.end()) {
        THROW_OR_ABORT("Cannot find meta material for \"" + physics_material_to_string(material));
    }
    return it->second;
}
