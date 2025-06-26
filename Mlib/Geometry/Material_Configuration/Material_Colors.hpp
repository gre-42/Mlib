#pragma once
#include <Mlib/Geometry/Material/Shading.hpp>
#include <cstdint>

namespace Mlib {

struct OsmResourceConfig;
enum class PhysicsMaterial: uint32_t;

// See the "plot_fresnel.py" script for tuning these parameters.

Shading material_shading(PhysicsMaterial material, const OsmResourceConfig& config);
Shading material_shading(const Shading& shading, const OsmResourceConfig& config);

namespace RawShading {
    using O = OrderableFixedArray<float, 3>;
    static const Shading CURB{ .specular = O(0.5f), .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = O(1.f)} };
    static const Shading MUD{ .specular = O(0.5f), .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = O(1.f)} };

    static const Shading ROOF{ .specular = O(0.2f), .fresnel = {.reflectance = {.min = 0.f, .max = 0.7f, .exponent = 5.f}, .ambient = O(1.f)} };
    static const Shading CEILING{ .specular = O(0.2f), .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = O(1.f)} };
    static const Shading DEFAULT{ .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = O(1.f)} };
}

}
