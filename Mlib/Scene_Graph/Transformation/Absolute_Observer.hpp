#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class AbsoluteMovable;

class AbsoluteObserver {
public:
    virtual ~AbsoluteObserver() = default;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) = 0;
};

}
