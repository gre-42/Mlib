#include "Material_Colors.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>
#include <map>

using namespace Mlib;

Shading Mlib::material_shading(PhysicsMaterial material) {
	auto mat = material & PhysicsMaterial::SURFACE_BASE_MASK;
	static const std::map<PhysicsMaterial, Shading> m{
		{PhysicsMaterial::NONE, { .emissive = {1.f, 0.f, 1.f}, .ambient = 0.f, .diffuse = 0.f, .specular = 0.f }},
		{PhysicsMaterial::SURFACE_BASE_TARMAC, { .ambient = 1.f, .specular = 0.5f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 4.f}} }},
		{PhysicsMaterial::SURFACE_BASE_GRAVEL, { .ambient = 1.f, .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}} }},
		{PhysicsMaterial::SURFACE_BASE_SNOW, { .ambient = 1.f, .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}} }},
		{PhysicsMaterial::SURFACE_BASE_ICE, { .ambient = 1.f, .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}} }},
		{PhysicsMaterial::SURFACE_BASE_SAND, { .ambient = 1.f, .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}} }},
		{PhysicsMaterial::SURFACE_BASE_GRASS, { .ambient = 1.f, .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.1f, .exponent = 10.f}} }},
		{PhysicsMaterial::SURFACE_BASE_DIRT, { .ambient = 1.f, .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.3f, .exponent = 5.f}} }},
		{PhysicsMaterial::SURFACE_BASE_STONE, { .ambient = 1.f, .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.15f, .exponent = 5.f}} }},
		{PhysicsMaterial::SURFACE_BASE_FOLIAGE, { .ambient = 1.f, .specular = 0.f, .fresnel = {.reflectance = {.min = 0.f, .max = 0.5f, .exponent = 5.f}} }}
	};
	auto it = m.find(mat);
	if (it == m.end()) {
		THROW_OR_ABORT("Cannot find shading options for material \"" + physics_material_to_string(mat));
	}
	return it->second;
}
