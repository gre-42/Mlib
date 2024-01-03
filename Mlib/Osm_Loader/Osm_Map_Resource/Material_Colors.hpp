#pragma once
#include <Mlib/Geometry/Material/Shading.hpp>

namespace Mlib {

static const OrderableFixedArray<float, 3> o3(float f) {
    return OrderableFixedArray<float, 3>(f);
}

static const Shading TARMAC_REFLECTANCE{ .specularity = 0.5f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };
static const Shading GRAVEL_REFLECTANCE{ .specularity = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f }} };
static const Shading SNOW_REFLECTANCE{ .specularity = 0.4f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };
static const Shading ICE_REFLECTANCE{ .specularity = 0.5f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };
static const Shading SAND_REFLECTANCE{ .specularity = 0.1f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };
static const Shading GRASS_REFLECTANCE{ .specularity = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };
static const Shading DIRT_REFLECTANCE{ .specularity = 0.2f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };
static const Shading STONE_REFLECTANCE{ .specularity = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };

static const Shading CURB_REFLECTANCE{ .specularity = 0.5f, .fresnel = {.reflectance = {.min = 0.3f, .max = 0.7f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };
static const Shading MUD_REFLECTANCE{ .specularity = 0.5f, .fresnel = {.reflectance = {.min = 0.3f, .max = 0.7f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };

static const Shading ROOF_REFLECTANCE{ .specularity = 0.2f, .fresnel = {.reflectance = {.min = 0.3f, .max = 0.7f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };
static const Shading CEILING_REFLECTANCE{ .specularity = 0.2f, .fresnel = {.reflectance = {.min = 0.3f, .max = 0.7f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };
static const Shading DEFAULT_REFLECTANCE{ .fresnel = {.reflectance = {.min = 0.3f, .max = 0.7f, .exponent = 4.f}, .ambience = {1.f, 1.f, 1.f}} };

}
