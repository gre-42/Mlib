#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/INode_Setter.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <memory>

namespace Mlib {

class AdvanceTimes;
class Scene;
class SceneNode;
class LookAtMovable;

class LookAtMovableFollowerNodeSetter: public INodeSetter {
public:
    explicit LookAtMovableFollowerNodeSetter(LookAtMovable& look_at_movable);
    virtual void set_scene_node(
        Scene& scene,
        const DanglingRef<SceneNode>& node,
        VariableAndHash<std::string> node_name,
        SourceLocation loc) override;
private:
    LookAtMovable& look_at_movable_;
    DestructionFunctionsRemovalTokens removal_tokens_;
};

class LookAtMovableFollowedNodeSetter: public INodeSetter {
public:
    explicit LookAtMovableFollowedNodeSetter(LookAtMovable& look_at_movable);
    virtual void set_scene_node(
        Scene& scene,
        const DanglingRef<SceneNode>& node,
        VariableAndHash<std::string> node_name,
        SourceLocation loc) override;
private:
    LookAtMovable& look_at_movable_;
    DestructionFunctionsRemovalTokens removal_tokens_;
};

class LookAtMovable: public IAbsoluteMovable, public IAdvanceTime, public virtual DanglingBaseClass {
    friend LookAtMovableFollowerNodeSetter;
    friend LookAtMovableFollowedNodeSetter;
public:
    LookAtMovable(
        AdvanceTimes& advance_times,
        Scene& scene,
        VariableAndHash<std::string> follower_name,
        DanglingRef<SceneNode> follower_node,
        DanglingRef<SceneNode> followed_node,
        IAbsoluteMovable& followed);
    virtual ~LookAtMovable() override;
    virtual void advance_time(float dt, const StaticWorld& world) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, ScenePos, 3> get_new_absolute_model_matrix() const override;
    LookAtMovableFollowerNodeSetter follower_setter;
    LookAtMovableFollowedNodeSetter followed_setter;

private:
    void notify_destroyed(SceneNode& destroyed_object);

    AdvanceTimes& advance_times_;
    Scene& scene_;
    VariableAndHash<std::string> follower_name_;
    DanglingPtr<SceneNode> follower_node_;
    DanglingPtr<SceneNode> followed_node_;
    IAbsoluteMovable* followed_;
    TransformationMatrix<float, ScenePos, 3> transformation_matrix_;
};

}
