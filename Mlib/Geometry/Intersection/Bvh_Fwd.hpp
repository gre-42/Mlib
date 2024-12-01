#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <cstdint>
#include <list>
#include <vector>

namespace Mlib {

enum class AabbExtensionDirection {
    BIDIRECTIONAL,
    POSITIVE
};

template <class TPosition, size_t tndim, class TData, AabbExtensionDirection extension_dir>
class GenericBvh;

template <class TPosition, size_t tndim, class TPayload>
class AabbAndPayload;

template <class TPosition, size_t tndim>
class PointWithoutPayload;

template <class TContainer>
class PayloadContainer;

template <class TPosition, class TPayload, size_t tndim>
using Bvh = GenericBvh<
    TPosition,
    tndim,
    PayloadContainer<std::list<AabbAndPayload<TPosition, tndim, TPayload>>>,
    AabbExtensionDirection::BIDIRECTIONAL>;

template <class TPosition, size_t tndim>
using PointVectorBvh = GenericBvh<
    TPosition,
    tndim,
    PayloadContainer<std::vector<PointWithoutPayload<TPosition, tndim>>>,
    AabbExtensionDirection::BIDIRECTIONAL>;

}
