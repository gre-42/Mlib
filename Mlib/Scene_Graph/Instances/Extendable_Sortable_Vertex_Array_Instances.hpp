#pragma once
#include <Mlib/Array/Chunked_Array.hpp>
#include <Mlib/Geometry/Billboard_Id.hpp>
#include <Mlib/Scene_Graph/Instances/Instance_Location.hpp>
#include <list>
#include <vector>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPosition>
struct PositionAndYAngleAndBillboardId;
template <class TPosition>
struct PositionAndBillboardId;
struct SortedVertexArrayInstances;

template <class TValue>
struct DistanceToCameraAndValue {
    float distance;
    TValue value;
};

template <class TValue>
using GenericSortableInstances = ChunkedArray<std::list<std::vector<DistanceToCameraAndValue<TValue>>>>;

using ExtendableSortableTransformedInstances = GenericSortableInstances<TransformationMatrix<float, float, 3>>;
using ExtendableSortableYAngleInstances = GenericSortableInstances<PositionAndYAngleAndBillboardId<float>>;
using ExtendableSortableLookatInstances = GenericSortableInstances<PositionAndBillboardId<float>>;

struct ExtendableSortableVertexArrayInstances {
    ExtendableSortableTransformedInstances transformed{1000};
    ExtendableSortableYAngleInstances yangle{1000};
    ExtendableSortableLookatInstances lookat{1000};
    void insert(const InstanceLocation& i, float distance, BillboardId billboard_id);
    SortedVertexArrayInstances sorted() const;
};

}
