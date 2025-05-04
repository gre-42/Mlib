#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Variable_And_Hash.hpp>

namespace Mlib {

struct Beacon {
    TransformationMatrix<float, ScenePos, 3> location;
    VariableAndHash<std::string> resource_name = VariableAndHash<std::string>{"beacon"};
    static Beacon create(
        const FixedArray<ScenePos, 3>& position,
        const VariableAndHash<std::string>& resource_name)
    {
        return Beacon{
            .location = TransformationMatrix<float, ScenePos, 3>{ fixed_identity_array<float, 3>(), position },
            .resource_name = resource_name};
    }
};

}
