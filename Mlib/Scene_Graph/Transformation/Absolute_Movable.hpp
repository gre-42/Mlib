#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

template <class TData>
class TransformationMatrix;

class AbsoluteMovable {
public:
    virtual ~AbsoluteMovable() = default;
    virtual void set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix) = 0;
    virtual TransformationMatrix<float> get_new_absolute_model_matrix() const = 0;
};

}
