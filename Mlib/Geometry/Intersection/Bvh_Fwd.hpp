#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <cstdint>
#include <list>
#include <vector>

namespace Mlib {

template <class TPosition, size_t tndim, class TData>
class GenericBvh;

template <class TPosition, size_t tndim, class TPayload>
class AabbAndPayload;

template <class TPosition, size_t tndim, class TPayload>
class PointAndPayload;

template <class TPosition, size_t tndim>
class PointWithoutPayload;

template <class TContainer>
class PayloadContainer;

template <class TPosition, size_t tndim, class TSmallContainer, class TLargeContainer>
class CompressedPayloadContainer;

template <class TPosition, size_t tndim, class TPayload>
using Bvh = GenericBvh<
    TPosition,
    tndim,
    PayloadContainer<std::list<AabbAndPayload<TPosition, tndim, TPayload>>>>;

template <class TPosition, size_t tndim, class TPayload>
using PointAndPayloadVectorBvh = GenericBvh<
    TPosition,
    tndim,
    PayloadContainer<std::vector<PointAndPayload<TPosition, tndim, TPayload>>>>;

template <class TPosition, size_t tndim>
using PointWithoutPayloadVectorBvh = GenericBvh<
    TPosition,
    tndim,
    PayloadContainer<std::vector<PointWithoutPayload<TPosition, tndim>>>>;

template <class TPosition, class TCompressedPosition, class TPayload, class TCompressedPayload, size_t tndim>
using CompressedBvh = GenericBvh<
    TPosition,
    tndim,
    CompressedPayloadContainer<
        TPosition,
        tndim,
        std::list<AabbAndPayload<TCompressedPosition, tndim, TCompressedPayload>>,
        std::list<AabbAndPayload<TPosition, tndim, TPayload>>>>;

template <class TPosition, class TCompressedPosition, size_t tndim>
using CompressedPointVectorBvh = GenericBvh<
    TPosition,
    tndim,
    CompressedPayloadContainer<
        TPosition,
        tndim,
        std::vector<PointWithoutPayload<TCompressedPosition, tndim>>,
        std::vector<PointWithoutPayload<TPosition, tndim>>>>;

}
