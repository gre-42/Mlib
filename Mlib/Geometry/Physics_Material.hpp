#pragma once
#include <string>

namespace Mlib {

static const size_t SURFACE_BASE_OFFSET = 18;
static const size_t SURFACE_BASE_NBITS = 4;

enum class PhysicsMaterial {
    NONE = 0,
    ATTR_VISIBLE = (1 << 0),
    ATTR_COLLIDE = (1 << 1),
    ATTR_TWO_SIDED = (1 << 2),
    ATTR_ALIGN_STRICT = (1 << 3),
    ATTR_CONVEX = (1 << 4),
    ATTR_CONCAVE = (1 << 5),
    OBJ_ALIGNMENT_PLANE = (1 << 7),
    OBJ_CHASSIS = (1 << 8),
    OBJ_TIRE_LINE = (1 << 9),
    OBJ_GRIND_CONTACT = (1 << 10),
    OBJ_GRIND_LINE = (1 << 11),
    OBJ_ALIGNMENT_CONTACT = (1 << 12),
    OBJ_BULLET_LINE_SEGMENT = (1 << 13),
    OBJ_BULLET_MESH = (1 << 14),
    OBJ_HITBOX = (1 << 15),
    OBJ_DISTANCEBOX = (1 << 16),
    OBJ_GRASS = (1 << 17),
    SURFACE_BASE_TARMAC = (1 << SURFACE_BASE_OFFSET),  //    1
    SURFACE_BASE_GRAVEL = (2 << SURFACE_BASE_OFFSET),  //   10
    SURFACE_BASE_SNOW = (3 << SURFACE_BASE_OFFSET),    //   11
    SURFACE_BASE_ICE = (4 << SURFACE_BASE_OFFSET),     //  100
    SURFACE_BASE_SAND = (5 << SURFACE_BASE_OFFSET),    //  101
    SURFACE_BASE_GRASS = (6 << SURFACE_BASE_OFFSET),   //  110
    SURFACE_BASE_DIRT = (7 << SURFACE_BASE_OFFSET),    //  111
    SURFACE_BASE_TIRE = (8 << SURFACE_BASE_OFFSET),    // 1000
    SURFACE_BASE_STONE = (9 << SURFACE_BASE_OFFSET),   // 1001
    SURFACE_MODIFIER_WET = (1 << (SURFACE_BASE_OFFSET + SURFACE_BASE_NBITS)),

    OBJ_BULLET_MASK = OBJ_BULLET_LINE_SEGMENT | OBJ_BULLET_MESH,
    OBJ_BULLET_COLLIDABLE_MASK = OBJ_CHASSIS | OBJ_HITBOX,

    SURFACE_BASE_MASK = ((1 << SURFACE_BASE_NBITS) - 1) << SURFACE_BASE_OFFSET
};

inline bool any(PhysicsMaterial a) {
    return a != PhysicsMaterial::NONE;
}

inline PhysicsMaterial operator & (PhysicsMaterial a, PhysicsMaterial b) {
    return (PhysicsMaterial)((unsigned int)a & (unsigned int)b);
}

inline PhysicsMaterial operator | (PhysicsMaterial a, PhysicsMaterial b) {
    return (PhysicsMaterial)((unsigned int)a | (unsigned int)b);
}

inline PhysicsMaterial& operator |= (PhysicsMaterial& a, PhysicsMaterial b) {
    a = a | b;
    return a;
}

inline PhysicsMaterial& operator &= (PhysicsMaterial& a, PhysicsMaterial b) {
    a = a & b;
    return a;
}

inline PhysicsMaterial operator ~ (PhysicsMaterial a) {
    return (PhysicsMaterial)(~(unsigned int)a);
}

PhysicsMaterial physics_material_from_string(const std::string& s);

}
