#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>

namespace Mlib {

class AdvanceTimes;
struct PhysicsEngineConfig;
struct RigidBodyIntegrator;

class PitchLookAtNode: public DestructionObserver, public RelativeMovable, public AdvanceTime {
public:
    PitchLookAtNode(
        SceneNode* followed_node,
        AdvanceTimes& advance_times,
        const RigidBodyIntegrator& follower,        
        const RigidBodyIntegrator* followed,
        float bullet_start_offset,
        float bullet_velocity,
        float gravity,
        float pitch_min,
        float pitch_max,
        float dpitch_max,
        float locked_on_max,
        const PhysicsEngineConfig& cfg);
    ~PitchLookAtNode();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, 3> get_new_relative_model_matrix() const override;
    virtual void notify_destroyed(void* obj) override;
    virtual void advance_time(float dt) override;
    void increment_pitch(float dpitch);
    void set_pitch(float pitch);

    void set_followed(SceneNode* followed_node, const RigidBodyIntegrator* followed);

    bool target_locked_on() const;

private:
    float pitch_;
    float pitch_min_;
    float pitch_max_;
    float dpitch_max_;
    float locked_on_max_;
    bool target_locked_on_;
    FixedArray<float, 3> relative_position_;
    SceneNode* followed_node_;
    AdvanceTimes& advance_times_;
    const RigidBodyIntegrator& follower_;
    const RigidBodyIntegrator* followed_;
    float bullet_start_offset_;
    float bullet_velocity_;
    float gravity_;
    const PhysicsEngineConfig& cfg_;
};

}
