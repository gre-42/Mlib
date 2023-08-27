#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>

namespace Mlib {

template <class T>
class DanglingRef;
class AdvanceTimes;
class RigidBodyVehicle;

class Wheel: public DestructionObserver<DanglingRef<const SceneNode>>, public RelativeMovable, public AdvanceTime {
public:
    explicit Wheel(
        RigidBodyVehicle& rigid_body,
        AdvanceTimes& advance_times,
        size_t tire_id,
        float radius);
    ~Wheel();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_relative_model_matrix() const override;
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(DanglingRef<const SceneNode> destroyed_object) override;
    RigidBodyVehicle& rigid_body_;
    AdvanceTimes& advance_times_;
    TransformationMatrix<float, double, 3> transformation_matrix_;
    size_t tire_id_;
    float angle_x_;
    float radius_;
    float y0_;
};

}
