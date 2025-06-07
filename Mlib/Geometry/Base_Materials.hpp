#pragma once
#include <Mlib/Geometry/Physics_Material.hpp>

namespace Mlib {

static const auto BASE_INVISIBLE_TERRAIN_MATERIAL =
    PhysicsMaterial::ATTR_COLLIDE |
    PhysicsMaterial::OBJ_CHASSIS |
    PhysicsMaterial::ATTR_CONCAVE;

static const auto BASE_VISIBLE_TERRAIN_MATERIAL = BASE_INVISIBLE_TERRAIN_MATERIAL | PhysicsMaterial::ATTR_VISIBLE;

static const auto BASE_WATER_MATERIAL =
    PhysicsMaterial::SURFACE_BASE_WATER |
    PhysicsMaterial::ATTR_LIQUID |
    PhysicsMaterial::ATTR_VISIBLE |
    PhysicsMaterial::ATTR_COLLIDE |
    PhysicsMaterial::ATTR_CONCAVE |
    PhysicsMaterial::ATTR_ANIMATED_COLOR |
    PhysicsMaterial::ATTR_ANIMATED_NORMAL;

}
