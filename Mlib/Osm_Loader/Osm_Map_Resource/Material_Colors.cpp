#include "Material_Colors.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <map>

using namespace Mlib;

Shading Mlib::material_shading(PhysicsMaterial material, const OsmResourceConfig& config) {
    const auto f0 = FresnelAndAmbient{.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 4.f}, .ambient = 1.f};
    const auto f1 = FresnelAndAmbient{.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}, .ambient = 1.f};
    const auto f2 = FresnelAndAmbient{.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = 1.f};
    const auto f3 = FresnelAndAmbient{.reflectance = {.min = 0.f, .max = 0.1f, .exponent = 10.f}, .ambient = 1.f};
    const auto f4 = FresnelAndAmbient{.reflectance = {.min = 0.f, .max = 0.5f, .exponent = 5.f}, .ambient = 1.f};
    const auto f5 = FresnelAndAmbient{.reflectance = {.min = 0.7f, .max = 1.f, .exponent = 5.f}, .ambient = 0.f};
    static const std::map<PhysicsMaterial, Shading> m{
        {PhysicsMaterial::NONE, { .emissive = {1.f, 0.f, 1.f}, .ambient = 0.f, .diffuse = 0.f, .specular = 0.f }},
        {PhysicsMaterial::SURFACE_BASE_TARMAC, { .specular = 0.5f, .fresnel = f0 }},
        {PhysicsMaterial::SURFACE_BASE_GRAVEL, { .specular = 0.f, .fresnel = f1 }},
        {PhysicsMaterial::SURFACE_BASE_SNOW, { .specular = 0.f, .fresnel = f1 }},
        {PhysicsMaterial::SURFACE_BASE_ICE, { .specular = 0.f, .fresnel = f1 }},
        {PhysicsMaterial::SURFACE_BASE_SAND, { .specular = 0.f, .fresnel = f2 }},
        {PhysicsMaterial::SURFACE_BASE_GRASS, { .specular = 0.f, .fresnel = f3 }},
        {PhysicsMaterial::SURFACE_BASE_DIRT, { .specular = 0.f, .fresnel = f1 }},
        {PhysicsMaterial::SURFACE_BASE_STONE, { .specular = 0.f, .fresnel = f2 }},
        {PhysicsMaterial::SURFACE_BASE_FOLIAGE, { .specular = 0.f, .fresnel = f4 }},
        {PhysicsMaterial::SURFACE_BASE_GLASS, { .specular = 0.f, .reflectance = 1.f, .fresnel = f5 }},
        {PhysicsMaterial::SURFACE_BASE_WATER | PhysicsMaterial::ATTR_ANIMATED_COLOR | PhysicsMaterial::ATTR_ANIMATED_NORMAL,
            { .diffuse = 1.f, .specular = 0.5f, .fresnel = f0 }},

        {PhysicsMaterial::SURFACE_BASE_TARMAC | PhysicsMaterial::SURFACE_WET, { .specular = 0.f, .reflectance = 1.f, .fresnel = f5 }},
        {PhysicsMaterial::SURFACE_BASE_DIRT | PhysicsMaterial::SURFACE_WET, { .specular = 0.f, .reflectance = 1.f, .fresnel = f5 }},
    };
    auto it = m.find(material);
    if (it == m.end()) {
        THROW_OR_ABORT("Cannot find shading options for material \"" + physics_material_to_string(material));
    }
    return material_shading(it->second, config);
}

Shading Mlib::material_shading(const Shading& shading, const OsmResourceConfig& config) {
    Shading res = shading;
    res.emissive *= config.emissive_factor;
    res.ambient *= config.ambient_factor;
    res.diffuse *= config.diffuse_factor;
    res.specular *= config.specular_factor;
    res.fresnel.ambient *= config.fresnel_ambient_factor;
    res.fog_distances = config.fog_distances;
    res.fog_ambient = config.fog_ambient;
    return res;
}
