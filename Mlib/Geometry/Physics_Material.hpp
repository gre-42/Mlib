#pragma once
#include <string>

namespace Mlib {

enum class PhysicsMaterial {
    NONE = 0,
    ATTR_VISIBLE = (1 << 0),
    ATTR_COLLIDE = (1 << 1),
    ATTR_TWO_SIDED = (1 << 2),
    ATTR_ALIGN_STRICT = (1 << 3),
    ATTR_CONVEX = (1 << 4),
    ATTR_CONCAVE = (1 << 5),
    OBJ_ALIGNMENT_PLANE = (1 << 6),
    OBJ_CHASSIS = (1 << 7),
    OBJ_TIRE_LINE = (1 << 8),
    OBJ_GRIND_CONTACT = (1 << 9),
    OBJ_GRIND_LINE = (1 << 10),
    OBJ_ALIGNMENT_CONTACT = (1 << 11),
    OBJ_BULLET_LINE_SEGMENT = (1 << 12),
    OBJ_BULLET_MESH = (1 << 13),
    OBJ_HITBOX = (1 << 14),
    OBJ_DISTANCEBOX = (1 << 15),

    OBJ_BULLET_MASK = OBJ_BULLET_LINE_SEGMENT | OBJ_BULLET_MESH,
    OBJ_BULLET_COLLIDABLE_MASK = OBJ_CHASSIS | OBJ_HITBOX
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
