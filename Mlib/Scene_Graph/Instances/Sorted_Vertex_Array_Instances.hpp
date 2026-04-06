#pragma once
#include <list>
#include <vector>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPosition>
struct PositionAndYAngleAndBillboardId;
template <class TPosition>
struct PositionAndBillboardId;

using SortedTransformedInstances = std::vector<TransformationMatrix<float, float, 3>>;
using SortedYAngleInstances = std::vector<PositionAndYAngleAndBillboardId<float>>;
using SortedLookatInstances = std::vector<PositionAndBillboardId<float>>;

struct SortedVertexArrayInstances {
    SortedTransformedInstances transformed;
    SortedYAngleInstances yangle;
    SortedLookatInstances lookat;
};

}
