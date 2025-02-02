#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <DetourNavMesh.h>
#include <cstddef>
#include <map>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t... tshape>
class OrderableFixedArray;

class Sample_SoloMesh;

class ShortestPathIntermediatePointsCreator {
public:
    explicit ShortestPathIntermediatePointsCreator(
        const Sample_SoloMesh& ssm,
        const std::map<OrderableFixedArray<CompressedScenePos, 3>, dtPolyRef>& poly_refs,
        float step_size);

    UUVector<FixedArray<CompressedScenePos, 3>> operator () (
        const FixedArray<CompressedScenePos, 3>& p0,
        const FixedArray<CompressedScenePos, 3>& p1) const;
private:
    const Sample_SoloMesh& ssm_;
    const std::map<OrderableFixedArray<CompressedScenePos, 3>, dtPolyRef>& poly_refs_;
    float step_size_;
};

}
