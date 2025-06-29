#pragma once
#include <Mlib/Geometry/Physics_Material.hpp>

namespace Mlib {

static const auto BASE_INVISIBLE_TERRAIN_MATERIAL =
    PhysicsMaterial::OBJ_CHASSIS |
    PhysicsMaterial::ATTR_COLLIDE |
    PhysicsMaterial::ATTR_CONCAVE;
static const auto BASE_VISIBLE_TERRAIN_MATERIAL = BASE_INVISIBLE_TERRAIN_MATERIAL | PhysicsMaterial::ATTR_VISIBLE;

static const auto BASE_VISIBLE_GROUND_MATERIAL =
    BASE_INVISIBLE_TERRAIN_MATERIAL |
    PhysicsMaterial::OBJ_GROUND |
    PhysicsMaterial::ATTR_VISIBLE;

static const auto BASE_VISIBLE_AIR_SUPPORT_MATERIAL =
    BASE_VISIBLE_TERRAIN_MATERIAL |
    PhysicsMaterial::OBJ_WAY_AIR_SUPPORT;

static const auto META_WATER_MATERIAL =
    PhysicsMaterial::OBJ_CHASSIS |
    PhysicsMaterial::ATTR_COLLIDE |
    PhysicsMaterial::ATTR_CONCAVE |
    PhysicsMaterial::ATTR_LIQUID |
    PhysicsMaterial::ATTR_ANIMATED_TEXTURES |
    PhysicsMaterial::ATTR_VISIBLE;

}
