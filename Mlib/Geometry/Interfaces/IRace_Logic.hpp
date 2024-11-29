#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <typename TData, size_t... tshape>
class FixedArray;

class IRaceLogic {
public:
    virtual void set_start_pose(
        const TransformationMatrix<float, ScenePos, 3>& pose,
        const FixedArray<float, 3>& velocity,
        const FixedArray<float, 3>& angular_velocity,
        unsigned int rank) = 0;
    virtual void set_checkpoints(
        const std::vector<TransformationMatrix<float, ScenePos, 3>>& checkpoints) = 0;
    virtual void set_circularity(bool is_circular) = 0;
};

}
