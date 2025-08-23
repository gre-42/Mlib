#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Interfaces/INode_Setter.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Signal/Kalman_Filter.hpp>
#include <memory>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class FollowMovable;

class FollowerMovableNodeSetter: public INodeSetter {
public:
    explicit FollowerMovableNodeSetter(FollowMovable& follow);
    virtual void set_scene_node(
        Scene& scene,
        const DanglingRef<SceneNode>& node,
        VariableAndHash<std::string> node_name,
        SourceLocation loc) override;
private:
    FollowMovable& follow_;
    DestructionFunctionsRemovalTokens removal_tokens_;
};

class FollowedMovableNodeSetter: public INodeSetter {
public:
    explicit FollowedMovableNodeSetter(FollowMovable& follow);
    virtual void set_scene_node(
        Scene& scene,
        const DanglingRef<SceneNode>& node,
        VariableAndHash<std::string> node_name,
        SourceLocation loc) override;
private:
    FollowMovable& follow_;
    DestructionFunctionsRemovalTokens removal_tokens_;
};

class FollowMovable: public IAbsoluteMovable, public IAdvanceTime, public virtual DanglingBaseClass {
    friend FollowerMovableNodeSetter;
    friend FollowedMovableNodeSetter;
public:
    FollowMovable(
        AdvanceTimes& advance_times,
        DanglingRef<const SceneNode> followed_node,
        IAbsoluteMovable& followed,
        float attachment_distance,
        const FixedArray<float, 3>& node_displacement,
        const FixedArray<float, 3>& look_at_displacement,
        float snappiness = 2.f,
        float y_adaptivity = 15.f,
        float y_snappiness = 0.05f,
        float dt = 1.f / 60.f * seconds,
        float dt_ref = 1.f / 60.f * seconds);
    virtual ~FollowMovable() override;
    void initialize(DanglingRef<SceneNode> follower_node);
    virtual void advance_time(float dt, const StaticWorld& world) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, ScenePos, 3> get_new_absolute_model_matrix() const override;
    FollowerMovableNodeSetter set_follower;
    FollowedMovableNodeSetter set_followed;

private:
    void notify_destroyed(SceneNode& destroyed_object);
    void advance_time(float dt);
    AdvanceTimes& advance_times_;
    DanglingPtr<const SceneNode> followed_node_;
    IAbsoluteMovable* followed_;
    float attachment_distance_;
    FixedArray<ScenePos, 2> attachment_position_;
    FixedArray<float, 3> node_displacement_;
    FixedArray<float, 3> look_at_displacement_;
    TransformationMatrix<float, ScenePos, 3> transformation_matrix_;
    FixedArray<ScenePos, 3> dpos_old_;
    float snappiness_;
    float y_adaptivity_;
    float y_adapt_;
    float dt_dt_ref_;
    KalmanFilter<float> kalman_filter_;
    ExponentialSmoother<float> exponential_smoother_;
    bool initialized_;
};

}
