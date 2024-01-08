#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class SceneNode;

class IRelativeMovable {
public:
    virtual ~IRelativeMovable() = default;
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) = 0;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) = 0;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) = 0;
    virtual TransformationMatrix<float, double, 3> get_new_relative_model_matrix() const = 0;
};

}
