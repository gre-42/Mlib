#pragma once
#include <Mlib/Geometry/Material/Shading.hpp>
#include <cstdint>

namespace Mlib {

enum class PhysicsMaterial: uint32_t;

// See the "plot_fresnel.py" script for tuning these parameters.

Shading material_shading(PhysicsMaterial material);

static const Shading CURB_REFLECTANCE{ .specular = 0.5f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = 1.f} };
static const Shading MUD_REFLECTANCE{ .specular = 0.5f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = 1.f} };

static const Shading ROOF_REFLECTANCE{ .specular = 0.2f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.7f, .exponent = 5.f}, .ambient = 1.f} };
static const Shading CEILING_REFLECTANCE{ .specular = 0.2f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = 1.f} };
static const Shading DEFAULT_REFLECTANCE{ .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = 1.f} };

}
