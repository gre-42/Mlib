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
class SceneNode;
class Scene;
class KeepOffsetFromMovable;

class KeepOffsetFromMovableFollowerNodeSetter: public INodeSetter {
public:
    explicit KeepOffsetFromMovableFollowerNodeSetter(KeepOffsetFromMovable& keep_offset);
    virtual void set_scene_node(
        Scene& scene,
        const DanglingBaseClassRef<SceneNode>& node,
        VariableAndHash<std::string> node_name,
        SourceLocation loc) override;
private:
    KeepOffsetFromMovable& keep_offset_;
    DestructionFunctionsRemovalTokens removal_tokens_;
};

class KeepOffsetFromMovableFollowedNodeSetter: public INodeSetter {
public:
    explicit KeepOffsetFromMovableFollowedNodeSetter(KeepOffsetFromMovable& keep_offset);
    virtual void set_scene_node(
        Scene& scene,
        const DanglingBaseClassRef<SceneNode>& node,
        VariableAndHash<std::string> node_name,
        SourceLocation loc) override;
private:
    KeepOffsetFromMovable& keep_offset_;
    DestructionFunctionsRemovalTokens removal_tokens_;
};

class KeepOffsetFromMovable: public IAbsoluteMovable, public IAdvanceTime, public virtual DanglingBaseClass {
    friend KeepOffsetFromMovableFollowerNodeSetter;
    friend KeepOffsetFromMovableFollowedNodeSetter;
public:
    KeepOffsetFromMovable(
        AdvanceTimes& advance_times,
        Scene& scene,
        VariableAndHash<std::string> follower_name,
        DanglingBaseClassRef<SceneNode> followed_node,
        IAbsoluteMovable& followed,
        const FixedArray<float, 3>& offset);
    virtual ~KeepOffsetFromMovable() override;
    virtual void advance_time(float dt, const StaticWorld& world) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, ScenePos, 3> get_new_absolute_model_matrix() const override;
    KeepOffsetFromMovableFollowerNodeSetter set_follower;
    KeepOffsetFromMovableFollowedNodeSetter set_followed;

private:
    void notify_destroyed(SceneNode& destroyed_object);
    AdvanceTimes& advance_times_;
    Scene& scene_;
    VariableAndHash<std::string> follower_name_;
    DanglingBaseClassPtr<SceneNode> followed_node_;
    IAbsoluteMovable* followed_;
    FixedArray<float, 3> offset_;
    TransformationMatrix<float, ScenePos, 3> transformation_matrix_;
};

}
