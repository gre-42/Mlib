#pragma once

namespace Mlib {

enum class PhysicsMaterial {
    SOLID = (1 << 0),
    ALIGNMENT_PLANE = (1 << 1),
    TWO_SIDED = (1 << 2)
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
