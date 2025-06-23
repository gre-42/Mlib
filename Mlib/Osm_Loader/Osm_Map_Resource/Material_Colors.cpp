#include "Material_Colors.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <map>

using namespace Mlib;

Shading Mlib::material_shading(PhysicsMaterial material, const OsmResourceConfig& config) {
    using O = OrderableFixedArray<float, 3>;
    const auto f0 = FresnelAndAmbient{.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 4.f}, .ambient = O(1.f)};
    const auto f1 = FresnelAndAmbient{.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}, .ambient = O(1.f)};
    const auto f2 = FresnelAndAmbient{.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = O(1.f)};
    const auto f3 = FresnelAndAmbient{.reflectance = {.min = 0.f, .max = 0.1f, .exponent = 10.f}, .ambient = O(1.f)};
    const auto f4 = FresnelAndAmbient{.reflectance = {.min = 0.f, .max = 0.5f, .exponent = 5.f}, .ambient = O(1.f)};
    const auto f5 = FresnelAndAmbient{.reflectance = {.min = 0.7f, .max = 1.f, .exponent = 5.f}, .ambient = O(0.f)};
    static const std::map<PhysicsMaterial, Shading> m{
        {PhysicsMaterial::NONE, Shading{ .emissive = O{1.f, 0.f, 1.f}, .ambient = O(0.f), .diffuse = O(0.f), .specular = O(0.f) }},
        {PhysicsMaterial::SURFACE_BASE_TARMAC, Shading{ .specular = O(0.5f), .fresnel = f0 }},
        {PhysicsMaterial::SURFACE_BASE_GRAVEL, Shading{ .specular = O(0.f), .fresnel = f1 }},
        {PhysicsMaterial::SURFACE_BASE_SNOW, Shading{ .specular = O(0.f), .fresnel = f1 }},
        {PhysicsMaterial::SURFACE_BASE_ICE, Shading{ .specular = O(0.f), .fresnel = f1 }},
        {PhysicsMaterial::SURFACE_BASE_SAND, Shading{ .specular = O(0.f), .fresnel = f2 }},
        {PhysicsMaterial::SURFACE_BASE_GRASS, Shading{ .specular = O(0.f), .fresnel = f3 }},
        {PhysicsMaterial::SURFACE_BASE_DIRT, Shading{ .specular = O(0.f), .fresnel = f1 }},
        {PhysicsMaterial::SURFACE_BASE_STONE, Shading{ .specular = O(0.f), .fresnel = f2 }},
        {PhysicsMaterial::SURFACE_BASE_FOLIAGE, Shading{ .specular = O(0.f), .fresnel = f4 }},
        {PhysicsMaterial::SURFACE_BASE_GLASS, Shading{ .specular = O(0.f), .reflectance = O(1.f), .fresnel = f5 }},
        {PhysicsMaterial::SURFACE_BASE_WATER, Shading{ .diffuse = O(1.f), .specular = O(0.5f), .fresnel = f0 }},
        {PhysicsMaterial::SURFACE_BASE_TARMAC | PhysicsMaterial::SURFACE_WET, Shading{ .specular = O(0.f), .reflectance = O(1.f), .fresnel = f5 }},
        {PhysicsMaterial::SURFACE_BASE_DIRT | PhysicsMaterial::SURFACE_WET, Shading{ .specular = O(0.f), .reflectance = O(1.f), .fresnel = f5 }},
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
