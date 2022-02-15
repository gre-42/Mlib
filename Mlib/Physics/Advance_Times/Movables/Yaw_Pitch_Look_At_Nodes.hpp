#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Relative_Movable.hpp>
#include <functional>

namespace Mlib {

class PitchLookAtNode;
class AdvanceTimes;
struct PhysicsEngineConfig;
struct RigidBodyVehicle;

class YawPitchLookAtNodes: public DestructionObserver, public RelativeMovable, public AdvanceTime {
public:
    YawPitchLookAtNodes(
        AdvanceTimes& advance_times,
        const RigidBodyVehicle& follower,
        float bullet_start_offset,
        float bullet_velocity,
        float gravity,
        float dyaw_max,
        float pitch_min,
        float pitch_max,
        float dpitch_max,
        float yaw_locked_on_max,
        float pitch_locked_on_max,
        const std::function<float()>& velocity_estimation_error,
        const PhysicsEngineConfig& cfg);
    ~YawPitchLookAtNodes();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, 3> get_new_relative_model_matrix() const override;
    virtual void notify_destroyed(void* obj) override;
    virtual void advance_time(float dt) override;
    void increment_yaw(float dyaw);
    void set_yaw(float yaw);

    void set_followed(
        SceneNode* followed_node,
        const RigidBodyVehicle* followed);
    std::shared_ptr<PitchLookAtNode> pitch_look_at_node() const;

    bool target_locked_on() const;

private:
    float dyaw_;
    float dyaw_max_;
    float yaw_locked_on_max_;
    bool yaw_target_locked_on_;
    TransformationMatrix<float, 3> relative_model_matrix_;
    SceneNode* followed_node_;
    AdvanceTimes& advance_times_;
    const RigidBodyVehicle& follower_;
    const RigidBodyVehicle* followed_;
    std::shared_ptr<PitchLookAtNode> pitch_look_at_node_;
    float bullet_start_offset_;
    float bullet_velocity_;
    float gravity_;
    std::function<float()> velocity_estimation_error_;
    const PhysicsEngineConfig& cfg_;
};

}
