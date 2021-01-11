#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>

namespace Mlib {

class AdvanceTimes;

class RelativeTransformer: public DestructionObserver, public RelativeMovable, public AdvanceTime {
public:
    explicit RelativeTransformer(AdvanceTimes& advance_times);
    ~RelativeTransformer();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float>& absolute_model_matrix) override;
    virtual TransformationMatrix<float> get_new_relative_model_matrix() const override;
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(void* obj) override;
    AdvanceTimes& advance_times_;
    TransformationMatrix<float> transformation_matrix_;
    FixedArray<float, 3> w_;
};

}
