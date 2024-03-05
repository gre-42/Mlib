#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <memory>

namespace Mlib {

class ITrailExtender;

struct TrailSource {
	std::unique_ptr<ITrailExtender> extender;
	FixedArray<float, 3> position;
	float minimum_velocity;
};

}
