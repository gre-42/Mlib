#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Collision/Resolve_Collision_Type.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Physics_Type.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>

namespace Mlib {

class AdvanceTimes;
class RigidBody;

class Wheel: public DestructionObserver, public RelativeMovable, public AdvanceTime {
public:
    explicit Wheel(
        RigidBody& rigid_body,
        AdvanceTimes& advance_times,
        size_t tire_id,
        float radius,
        PhysicsType physics_type,
        ResolveCollisionType resolve_collision_type);
    ~Wheel();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float>& absolute_model_matrix) override;
    virtual TransformationMatrix<float> get_new_relative_model_matrix() const override;
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(void* obj) override;
    RigidBody& rigid_body_;
    AdvanceTimes& advance_times_;
    TransformationMatrix<float> transformation_matrix_;
    size_t tire_id_;
    float angle_x_;
    float radius_;
    float y0_;
    PhysicsType physics_type_;
    ResolveCollisionType resolve_collision_type_;
};

}
