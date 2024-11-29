#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class IAbsoluteObserver {
public:
    virtual ~IAbsoluteObserver() = default;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) = 0;
};

}
