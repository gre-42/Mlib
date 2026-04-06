
#include "Instance_Location.hpp"
#include <Mlib/Geometry/Material/Transformation_Mode.hpp>
#include <Mlib/Math/Transformation/Transformation_Variant.hpp>
#include <stdexcept>

using namespace Mlib;

InstanceLocation Mlib::instance_location_from_transformation(
    const TransformationMatrix<SceneDir, ScenePos, 3>& matrix,
    TransformationMode mode)
{
    switch (mode) {
    case TransformationMode::ALL:
        return {matrix};
    case TransformationMode::POSITION_FLAT:
    case TransformationMode::POSITION_LOOKAT:
    case TransformationMode::POSITION:
        return {TranslationMatrix{matrix.t}};
    case TransformationMode::POSITION_YANGLE:
        return {OffsetAndYAngle{matrix.t, std::atan2(-matrix.R(2, 0), matrix.R(0, 0))}};
    }
    throw std::runtime_error("Unknown transformation mode: " +  std::to_string((int)mode));
}
