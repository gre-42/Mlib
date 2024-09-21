#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IRelative_Movable.hpp>
#include <functional>

namespace Mlib {

template <class T>
class DanglingRef;
class PitchLookAtNode;
struct PhysicsEngineConfig;
class RigidBodyVehicle;
class AimAt;
class SceneNode;

class YawPitchLookAtNodes: public DestructionObserver<SceneNode&>, public IRelativeMovable, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    YawPitchLookAtNodes(
        AimAt& aim_at,
        PitchLookAtNode& pitch_look_at_node,
        float dyaw_max,
        const std::function<float()>& increment_yaw_error);
    ~YawPitchLookAtNodes();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, ScenePos, 3> get_new_relative_model_matrix() const override;
    virtual void notify_destroyed(SceneNode& destroyed_object) override;
    virtual void advance_time(float dt, const StaticWorld& world) override;
    void increment_yaw(float dyaw, float relaxation);
    void set_yaw(float yaw);

    PitchLookAtNode& pitch_look_at_node() const;

private:
    AimAt& aim_at_node_;
    float dyaw_;
    float dyaw_max_;
    TransformationMatrix<float, ScenePos, 3> relative_model_matrix_;
    PitchLookAtNode& pitch_look_at_node_;
    std::function<float()> increment_yaw_error_;
};

}
