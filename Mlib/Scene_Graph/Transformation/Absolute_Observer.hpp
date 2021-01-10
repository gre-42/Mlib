#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

template <class TData>
class TransformationMatrix;
class AbsoluteMovable;

class AbsoluteObserver {
public:
    virtual ~AbsoluteObserver() = default;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float>& absolute_model_matrix) = 0;
};

}
