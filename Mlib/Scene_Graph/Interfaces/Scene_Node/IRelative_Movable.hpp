#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Observer.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class IRelativeMovable: public IAbsoluteObserver {
public:
    virtual ~IRelativeMovable() = default;
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) = 0;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) = 0;
    virtual TransformationMatrix<float, ScenePos, 3> get_new_relative_model_matrix() const = 0;
};

}
