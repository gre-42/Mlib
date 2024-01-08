#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IRelative_Movable.hpp>
#include <functional>

namespace Mlib {

class PitchLookAtNode;
class AdvanceTimes;
struct PhysicsEngineConfig;
class RigidBodyVehicle;

class YawPitchLookAtNodes: public DestructionObserver<DanglingRef<const SceneNode>>, public IRelativeMovable, public AdvanceTime {
public:
    YawPitchLookAtNodes(
        PitchLookAtNode& pitch_look_at_node,
        AdvanceTimes& advance_times,
        const RigidBodyVehicle& follower,
        float bullet_start_offset,
        float bullet_velocity,
        bool bullet_feels_gravity,
        float gravity,
        float dyaw_max,
        float yaw_locked_on_max,
        const std::function<float()>& velocity_estimation_error,
        const std::function<float()>& increment_yaw_error,
        const std::function<float()>& increment_pitch_error);
    ~YawPitchLookAtNodes();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_relative_model_matrix() const override;
    virtual void notify_destroyed(DanglingRef<const SceneNode> destroyed_object) override;
    virtual void advance_time(float dt) override;
    void increment_yaw(float dyaw);
    void set_yaw(float yaw);

    void set_followed(
        DanglingPtr<SceneNode> followed_node,
        const RigidBodyVehicle* followed);
    PitchLookAtNode& pitch_look_at_node() const;

    bool target_locked_on() const;
    void set_bullet_velocity(float value);
    void set_bullet_feels_gravity(bool value);

private:
    float dyaw_;
    float dyaw_max_;
    float yaw_locked_on_max_;
    bool yaw_target_locked_on_;
    TransformationMatrix<float, double, 3> relative_model_matrix_;
    DanglingPtr<SceneNode> followed_node_;
    AdvanceTimes& advance_times_;
    const RigidBodyVehicle& follower_;
    const RigidBodyVehicle* followed_;
    PitchLookAtNode& pitch_look_at_node_;
    float bullet_start_offset_;
    float bullet_velocity_;
    bool bullet_feels_gravity_;
    float gravity_;
    std::function<float()> velocity_estimation_error_;
    std::function<float()> increment_yaw_error_;
};

}
