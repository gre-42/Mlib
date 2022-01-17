#pragma once
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Actuators/Base_Rotor.hpp>
#include <string>

namespace Mlib {

struct Rotor: public BaseRotor {
    Rotor(
        const std::string& engine,
        const TransformationMatrix<float, 3>& rest_location,
        float power2lift);
    TransformationMatrix<float, 3> rotated_location() const;
    TransformationMatrix<float, 3> rest_location;
    FixedArray<float, 3> angles;
    float power2lift;
};

}
