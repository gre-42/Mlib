#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t tndim>
class TransformationVariant;
enum class TransformationMode;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

using InstanceLocation = TransformationVariant<SceneDir, ScenePos, 3>;

InstanceLocation instance_location_from_transformation(
    const TransformationMatrix<SceneDir, ScenePos, 3>& matrix,
    TransformationMode mode);

}
