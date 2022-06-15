#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class AbsoluteMovable {
public:
    virtual ~AbsoluteMovable() = default;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) = 0;
    virtual TransformationMatrix<float, double, 3> get_new_absolute_model_matrix() const = 0;
};

}
