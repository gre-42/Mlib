#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <chrono>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class IParticleCreator {
public:
    virtual ~IParticleCreator() = default;
    virtual void add_particle(
        std::chrono::steady_clock::time_point time,
        const TransformationMatrix<float, ScenePos, 3>& transformation_matrix,
        const FixedArray<float, 3>& velocity,
        float air_resistance_halflife,
        float texture_layer) = 0;
    
};

}
