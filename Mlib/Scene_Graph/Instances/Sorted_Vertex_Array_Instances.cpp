#include "Sorted_Vertex_Array_Instances.hpp"
#include <Mlib/Geometry/Material/Transformation_Mode.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Scene_Graph/Instances/Billboard_Container.hpp>
#include <stdexcept>

using namespace Mlib;

size_t SortedVertexArrayInstances::size(TransformationMode transformation_mode) const {
    switch (transformation_mode) {
    case TransformationMode::ALL:
        return transformed.size();
    case TransformationMode::POSITION_FLAT:
    case TransformationMode::POSITION_LOOKAT:
    case TransformationMode::POSITION:
        return lookat.size();
    case TransformationMode::POSITION_YANGLE:
        return yangle.size();
    }
    throw std::runtime_error("SortedVertexArrayInstances: Unsupported transformation mode");
}
