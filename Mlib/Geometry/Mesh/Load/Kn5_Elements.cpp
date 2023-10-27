#include "Kn5_Elements.hpp"
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

float kn5Material::mult(size_t i) const {
	switch (i) {
	case 0:
		return multR.value_or_default();
	case 1:
		return multG.value_or_default();
	case 2:
		return multB.value_or_default();
	case 3:
		return multA.value_or_default();
	default:
		THROW_OR_ABORT("kn5Material::mult index out of bounds");
	}
}
