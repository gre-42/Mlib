#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class IAbsoluteMovable {
public:
    virtual ~IAbsoluteMovable() = default;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) = 0;
    virtual TransformationMatrix<float, ScenePos, 3> get_new_absolute_model_matrix() const = 0;
};

}
