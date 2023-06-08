#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>
#include <memory>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class Scene;

class CopyRotation: public DestructionObserver, public RelativeMovable, public AdvanceTime {
public:
    CopyRotation(
        AdvanceTimes& advance_times,
        SceneNode& from);
    ~CopyRotation();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_relative_model_matrix() const override;
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(const Object& destroyed_object) override;
private:
    AdvanceTimes& advance_times_;
    SceneNode* from_;
    TransformationMatrix<float, double, 3> transformation_matrix_;
};

}
