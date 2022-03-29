#pragma once
#include <cstddef>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class Sample_SoloMesh;

class ShortestPathIntermediatePointsCreator {
public:
    explicit ShortestPathIntermediatePointsCreator(const Sample_SoloMesh& ssm);

    std::vector<FixedArray<float, 3>> operator () (
        const FixedArray<float, 3>& p0,
        const FixedArray<float, 3>& p1,
        const float& distance) const;
private:
    const Sample_SoloMesh& ssm_;
};

}
