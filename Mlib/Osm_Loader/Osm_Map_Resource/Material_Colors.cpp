#include "Material_Colors.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <map>

using namespace Mlib;

Shading Mlib::material_shading(PhysicsMaterial material) {
	auto mat = material & PhysicsMaterial::SURFACE_BASE_MASK;
	static const std::map<PhysicsMaterial, Shading> m{
		{PhysicsMaterial::NONE, { .emissivity = {1.f, 0.f, 1.f}, .ambience = 0.f, .diffusivity = 0.f, .specularity = 0.f }},
		{PhysicsMaterial::SURFACE_BASE_TARMAC, { .specularity = 0.5f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 4.f}, .ambience = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_GRAVEL, { .specularity = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}, .ambience = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_SNOW, { .specularity = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}, .ambience = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_ICE, { .specularity = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}, .ambience = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_SAND, { .specularity = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambience = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_GRASS, { .specularity = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.1f, .exponent = 5.f}, .ambience = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_DIRT, { .specularity = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}, .ambience = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_STONE, { .specularity = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambience = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_FOLIAGE, { .specularity = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.5f, .exponent = 5.f}, .ambience = 1.f} }}
	};
	auto it = m.find(mat);
	if (it == m.end()) {
		THROW_OR_ABORT("Cannot find shading options for material \"" + physics_material_to_string(mat));
	}
	return it->second;
}
