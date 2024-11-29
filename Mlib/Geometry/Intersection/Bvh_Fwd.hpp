#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <list>
#include <vector>

namespace Mlib {

template <class TPosition, size_t tndim, class TData>
class GenericBvh;

template <class TPosition, size_t tndim, class TPayload>
class AabbAndPayload;

template <class TPosition, size_t tndim, class TPayload>
class PointAndPayload;

template <class TContainer>
class PayloadContainer;

template <class TPosition, class TPayload, size_t tndim>
using Bvh = GenericBvh<
    TPosition,
    tndim,
    PayloadContainer<std::list<AabbAndPayload<TPosition, tndim, TPayload>>>>;

template <class TPosition, class TPayload, size_t tndim>
using PointVectorBvh = GenericBvh<
    TPosition,
    tndim,
    std::vector<PointAndPayload<TPosition, tndim, TPayload>>>;

}
