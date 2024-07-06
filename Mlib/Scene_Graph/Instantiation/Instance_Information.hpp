#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <string>

namespace Mlib {

struct InstanceInformation {
	std::string resource_name;
	TransformationMatrix<float, double, 3> trafo;
};

}
