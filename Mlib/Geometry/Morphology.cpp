#include "Morphology.hpp"
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

Morphology Mlib::operator + (const Morphology& a, PhysicsMaterial b) {
	Morphology result = a;
	result.physics_material |= b;
	return result;
}

Morphology Mlib::operator - (const Morphology& a, PhysicsMaterial value) {
	Morphology result = a;
	result.physics_material &= ~value;
	return result;
}
