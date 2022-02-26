#pragma once

namespace Mlib {

enum class PhysicsMaterial {
    NONE = 0,
    ATTR_VISIBLE = (1 << 0),
    ATTR_COLLIDE = (1 << 1),
    ATTR_TWO_SIDED = (1 << 2),
    OBJ_ALIGNMENT_PLANE = (1 << 3),
    OBJ_CHASSIS = (1 << 4),
    OBJ_TIRE_LINE = (1 << 5),
    OBJ_GRIND_CONTACT = (1 << 6),
    OBJ_GRIND_LINE = (1 << 7),
    OBJ_ALIGNMENT_CONTACT = (1 << 8)
};

inline bool operator & (PhysicsMaterial a, PhysicsMaterial b) {
    return ((unsigned int)a & (unsigned int)b) != 0;
}

inline PhysicsMaterial operator | (PhysicsMaterial a, PhysicsMaterial b) {
    return (PhysicsMaterial)((unsigned int)a | (unsigned int)b);
}

inline PhysicsMaterial& operator |= (PhysicsMaterial& a, PhysicsMaterial b) {
    a = a | b;
    return a;
}

}
