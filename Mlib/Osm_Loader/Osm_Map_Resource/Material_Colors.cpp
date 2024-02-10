#include "Material_Colors.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <map>

using namespace Mlib;

Shading Mlib::material_shading(PhysicsMaterial material) {
	auto mat = material & PhysicsMaterial::SURFACE_BASE_MASK;
	static const std::map<PhysicsMaterial, Shading> m{
		{PhysicsMaterial::NONE, { .emissive = {1.f, 0.f, 1.f}, .ambient = 0.f, .diffuse = 0.f, .specular = 0.f }},
		{PhysicsMaterial::SURFACE_BASE_TARMAC, { .specular = 0.5f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 4.f}, .ambient = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_GRAVEL, { .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}, .ambient = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_SNOW, { .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}, .ambient = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_ICE, { .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}, .ambient = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_SAND, { .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_GRASS, { .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.1f, .exponent = 10.f}, .ambient = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_DIRT, { .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}, .ambient = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_STONE, { .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}, .ambient = 1.f} }},
		{PhysicsMaterial::SURFACE_BASE_FOLIAGE, { .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.5f, .exponent = 5.f}, .ambient = 1.f} }}
	};
	auto it = m.find(mat);
	if (it == m.end()) {
		THROW_OR_ABORT("Cannot find shading options for material \"" + physics_material_to_string(mat));
	}
	return it->second;
}
