#pragma once
#include <DetourNavMesh.h>
#include <cstddef>
#include <map>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <typename TData, size_t... tshape>
class OrderableFixedArray;

class Sample_SoloMesh;

class ShortestPathIntermediatePointsCreator {
public:
    explicit ShortestPathIntermediatePointsCreator(
        const Sample_SoloMesh& ssm,
        const std::map<OrderableFixedArray<float, 3>, dtPolyRef>& poly_refs,
        float step_size);

    std::vector<FixedArray<float, 3>> operator () (
        const FixedArray<float, 3>& p0,
        const FixedArray<float, 3>& p1,
        const float& distance) const;
private:
    const Sample_SoloMesh& ssm_;
    const std::map<OrderableFixedArray<float, 3>, dtPolyRef>& poly_refs_;
    float step_size_;
};

}
