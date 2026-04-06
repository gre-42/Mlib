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
using GenericExtendableInstances = ChunkedArray<std::list<std::vector<TValue>>>;

using ExtendableTransformedInstances = GenericExtendableInstances<TransformationMatrix<float, float, 3>>;
using ExtendableYAngleInstances = GenericExtendableInstances<PositionAndYAngleAndBillboardId<float>>;
using ExtendableLookatInstances = GenericExtendableInstances<PositionAndBillboardId<float>>;

struct ExtendableVertexArrayInstances {
    ExtendableTransformedInstances transformed{1000};
    ExtendableYAngleInstances yangle{1000};
    ExtendableLookatInstances lookat{1000};
    void insert(const InstanceLocation& i, BillboardId billboard_id);
    SortedVertexArrayInstances vectorized() const;
};

}
