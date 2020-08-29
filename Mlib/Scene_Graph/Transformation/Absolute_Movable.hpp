#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

class AbsoluteMovable {
public:
    virtual ~AbsoluteMovable() = default;
    virtual void set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix) = 0;
    virtual FixedArray<float, 4, 4> get_new_absolute_model_matrix() const = 0;
};

}
