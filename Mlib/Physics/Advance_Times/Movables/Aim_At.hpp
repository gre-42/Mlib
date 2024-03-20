#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Guards.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Observer.hpp>
#include <functional>

namespace Mlib {

class PitchLookAtNode;
class AdvanceTimes;
struct PhysicsEngineConfig;
class SceneNode;
class RigidBodyVehicle;

class AimAt: public IAbsoluteObserver, public AdvanceTime {
public:
    AimAt(
        AdvanceTimes& advance_times,
        DanglingRef<SceneNode> follower_node,
        DanglingRef<SceneNode> gun_node,
        float bullet_start_offset,
        float bullet_velocity,
        bool bullet_feels_gravity,
        float gravity,
        float locked_on_cosine_min,
        const std::function<float()>& velocity_estimation_error);
    ~AimAt();
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual void advance_time(float dt) override;

    bool has_followed() const;
    void set_followed(DanglingPtr<SceneNode> followed_node);

    bool target_locked_on() const;
    void set_bullet_velocity(float value);
    void set_bullet_feels_gravity(bool value);

    const FixedArray<double, 3>& point_to_aim_at() const;

private:
    FixedArray<double, 3> point_to_aim_at_;
    DanglingPtr<SceneNode> followed_node_;
    AdvanceTimes& advance_times_;
    const RigidBodyVehicle& follower_;
    const RigidBodyVehicle* followed_;
    float bullet_start_offset_;
    float bullet_velocity_;
    bool bullet_feels_gravity_;
    float gravity_;
    float locked_on_cosine_min_;
    bool target_locked_on_;
    std::function<float()> velocity_estimation_error_;
    DestructionGuards dgs_;
    DestructionFunctionsRemovalTokens follower_node_on_destroy_;
    std::optional<DestructionFunctionsRemovalTokens> followed_node_on_destroy_;
};

}
