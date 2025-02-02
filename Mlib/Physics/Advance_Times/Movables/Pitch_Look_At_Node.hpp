#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IRelative_Movable.hpp>
#include <functional>

namespace Mlib {

struct PhysicsEngineConfig;
class RigidBodyVehicle;
class AimAt;
class SceneNode;

class PitchLookAtNode: public DestructionObserver<SceneNode&>, public IRelativeMovable, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    PitchLookAtNode(
        AimAt& aim_at,
        float pitch_min,
        float pitch_max,
        float dpitch_max,
        const std::function<float()>& increment_pitch_error);
    virtual ~PitchLookAtNode() override;
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, ScenePos, 3> get_new_relative_model_matrix() const override;
    virtual void notify_destroyed(SceneNode& destroyed_object) override;
    virtual void advance_time(float dt, const StaticWorld& world) override;
    void increment_pitch(float dpitch, float relaxation);
    void set_pitch(float pitch);

    void set_head_node(DanglingRef<SceneNode> head_node);

    void set_dpitch_head(float value);
    float get_dpitch_head() const;

private:
    AimAt& aim_at_node_;
    float dpitch_;
    float pitch_;
    float pitch_min_;
    float pitch_max_;
    float dpitch_max_;
    FixedArray<ScenePos, 3> relative_position_;
    float dpitch_head_;
    DanglingPtr<SceneNode> head_node_;
    std::function<float()> increment_pitch_error_;
};

}
