#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>
#include <functional>

namespace Mlib {

class AdvanceTimes;
struct PhysicsEngineConfig;
class RigidBodyVehicle;

class PitchLookAtNode: public DestructionObserver, public RelativeMovable, public AdvanceTime {
public:
    PitchLookAtNode(
        AdvanceTimes& advance_times,
        const RigidBodyVehicle& follower,        
        float bullet_start_offset,
        float bullet_velocity,
        bool bullet_feels_gravity,
        float gravity,
        float pitch_min,
        float pitch_max,
        float dpitch_max,
        float locked_on_max,
        const std::function<float()>& velocity_estimation_error,
        const std::function<float()>& increment_pitch_error);
    ~PitchLookAtNode();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_relative_model_matrix() const override;
    virtual void notify_destroyed(void* obj) override;
    virtual void advance_time(float dt) override;
    void increment_pitch(float dpitch);
    void set_pitch(float pitch);

    void set_followed(
        SceneNode* followed_node,
        const RigidBodyVehicle* followed);
    void set_head_node(SceneNode& head_node);

    bool target_locked_on() const;
    void set_bullet_velocity(float value);
    void set_bullet_feels_gravity(bool value);
    void set_dpitch_head(float value);
    float get_dpitch_head() const;

private:
    float pitch_;
    float pitch_min_;
    float pitch_max_;
    float dpitch_max_;
    float locked_on_max_;
    bool target_locked_on_;
    FixedArray<double, 3> relative_position_;
    SceneNode* followed_node_;
    AdvanceTimes& advance_times_;
    const RigidBodyVehicle& follower_;
    const RigidBodyVehicle* followed_;
    float bullet_start_offset_;
    float bullet_velocity_;
    bool bullet_feels_gravity_;
    float gravity_;
    float dpitch_head_;
    SceneNode* head_node_;
    std::function<float()> velocity_estimation_error_;
    std::function<float()> increment_pitch_error_;
};

}
