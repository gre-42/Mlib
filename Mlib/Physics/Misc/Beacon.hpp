#pragma once
#include <Mlib/Math/Transformation_Matrix.hpp>

namespace Mlib {

struct Beacon {
    TransformationMatrix<float, 3> location;
    std::string resource_name = "beacon";
    static Beacon create(
        const FixedArray<float, 3>& position,
        const std::string& resource_name)
    {
        return Beacon{
            .location = TransformationMatrix<float, 3>{ fixed_identity_array<float, 3>(), position },
            .resource_name = resource_name};
    }
};

}
