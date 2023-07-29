#pragma once
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class IRaceLogic {
public:
    virtual void set_start_pose(
        const TransformationMatrix<float, double, 3>& pose,
        unsigned int rank) = 0;
};

}
