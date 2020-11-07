#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Physics_Type.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>

namespace Mlib {

class AdvanceTimes;
class RigidBody;

class Wheel: public DestructionObserver, public RelativeMovable, public AdvanceTime {
public:
    explicit Wheel(RigidBody& rigid_body, AdvanceTimes& advance_times, size_t tire_id, float radius, PhysicsType physics_type);
    virtual void set_initial_relative_model_matrix(const FixedArray<float, 4, 4>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const FixedArray<float, 4, 4>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix) override;
    virtual FixedArray<float, 4, 4> get_new_relative_model_matrix() const override;
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(void* obj) override;
    RigidBody& rigid_body_;
    AdvanceTimes& advance_times_;
    FixedArray<float, 3> position_;
    FixedArray<float, 3, 3> rotation_;
    size_t tire_id_;
    float angle_x_;
    float radius_;
    float y0_;
    PhysicsType physics_type_;
};

}
