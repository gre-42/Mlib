#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <string>

namespace Mlib {

enum class RenderingDynamics;

template <class TPosition>
struct InstanceInformation {
    VariableAndHash<std::string> resource_name;
    TransformationMatrix<float, TPosition, 3> trafo;
    float scale;
    RenderingDynamics rendering_dynamics;
};

}
