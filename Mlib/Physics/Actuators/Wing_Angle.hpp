#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>

namespace Mlib {

class WingAngle: public RelativeMovable {
public:
    explicit WingAngle(float& angle, const FixedArray<float, 3>& rotation_axis);
    // RelativeMovable
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_relative_model_matrix() const override;
private:
    float& angle_;
    FixedArray<float, 3> rotation_axis_;
    FixedArray<double, 3> position_;
};

}
