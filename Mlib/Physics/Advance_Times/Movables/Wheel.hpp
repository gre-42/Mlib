#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IRelative_Movable.hpp>

namespace Mlib {

template <class T>
class DanglingRef;
class SceneNode;
class AdvanceTimes;
class RigidBodyVehicle;

class Wheel: public DestructionObserver<SceneNode&>, public IRelativeMovable, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    explicit Wheel(
        RigidBodyVehicle& rigid_body,
        AdvanceTimes& advance_times,
        size_t tire_id,
        float radius);
    ~Wheel();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, ScenePos, 3> get_new_relative_model_matrix() const override;
    virtual void advance_time(float dt, const StaticWorld& world) override;
    virtual void notify_destroyed(SceneNode& destroyed_object) override;
    RigidBodyVehicle& rigid_body_;
    AdvanceTimes& advance_times_;
    TransformationMatrix<float, ScenePos, 3> transformation_matrix_;
    size_t tire_id_;
    float angle_x_;
    float radius_;
    float y0_;
};

}
