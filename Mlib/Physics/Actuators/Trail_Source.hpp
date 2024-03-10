#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <memory>

namespace Mlib {

class ITrailExtender;

struct TrailSource {
	inline TrailSource(
		std::unique_ptr<ITrailExtender> extender,
		const FixedArray<float, 3> position,
		float minimum_velocity)
	: extender{ std::move(extender) },
	  position{ position },
	  minimum_velocity{ minimum_velocity }
	{}
	std::unique_ptr<ITrailExtender> extender;
	FixedArray<float, 3> position;
	float minimum_velocity;
};

}
