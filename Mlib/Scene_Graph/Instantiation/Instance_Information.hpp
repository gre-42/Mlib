#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <string>

namespace Mlib {

enum class RenderingDynamics;

struct InstanceInformation {
	std::string resource_name;
	TransformationMatrix<float, double, 3> trafo;
	float scale;
	RenderingDynamics rendering_dynamics;
};

}
