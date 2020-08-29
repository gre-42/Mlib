#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

class AbsoluteMovable;

class AbsoluteObserver {
public:
    virtual ~AbsoluteObserver() = default;
    virtual void set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix) = 0;
};

}
