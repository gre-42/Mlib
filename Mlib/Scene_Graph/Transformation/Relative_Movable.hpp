#pragma once
#include <Mlib/Array/Array_Forward.hpp>

namespace Mlib {

class SceneNode;

class RelativeMovable {
public:
    virtual ~RelativeMovable() = default;
    virtual void set_initial_relative_model_matrix(const FixedArray<float, 4, 4>& relative_model_matrix) = 0;
    virtual void set_updated_relative_model_matrix(const FixedArray<float, 4, 4>& relative_model_matrix) = 0;
    virtual void set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix) = 0;
    virtual FixedArray<float, 4, 4> get_new_relative_model_matrix() const = 0;
};

}
