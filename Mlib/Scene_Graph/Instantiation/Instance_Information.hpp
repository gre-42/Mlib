#pragma once
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <string>

namespace Mlib {

enum class RenderingDynamics;

struct InstanceInformation {
    std::string resource_name;
    TransformationMatrix<float, ScenePos, 3> trafo;
    float scale;
    RenderingDynamics rendering_dynamics;
};

}
