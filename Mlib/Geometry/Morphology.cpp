#include "Morphology.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

Morphology& Mlib::operator += (Morphology& a, PhysicsMaterial b) {
    a.physics_material |= b;
    return a;
}

Morphology& Mlib::operator -= (Morphology& a, PhysicsMaterial b) {
    a.physics_material &= ~b;
    return a;
}

Morphology Mlib::operator + (const Morphology& a, PhysicsMaterial b) {
    Morphology result = a;
    result += b;
    return result;
}

Morphology Mlib::operator - (const Morphology& a, PhysicsMaterial b) {
    Morphology result = a;
    result -= b;
    return result;
}
