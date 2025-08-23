#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/INode_Setter.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <memory>

namespace Mlib {

template <class T>
class DanglingRef;
class AdvanceTimes;
class SceneNode;
class Scene;
class SelectedCameras;
class EventReceiverDeletionToken;

class KeepOffsetFromCamera: public INodeSetter, public IAbsoluteMovable, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    KeepOffsetFromCamera(
        AdvanceTimes& advance_times,
        Scene& scene,
        SelectedCameras& cameras,
        const FixedArray<float, 3>& offset,
        const FixedArray<float, 3>& grid,
        const DanglingRef<SceneNode>& follower_node);
    ~KeepOffsetFromCamera();
    virtual void advance_time(float dt, const StaticWorld& world) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, ScenePos, 3> get_new_absolute_model_matrix() const override;
    virtual void set_scene_node(
        Scene& scene,
        const DanglingRef<SceneNode>& node,
        VariableAndHash<std::string> node_name,
        SourceLocation loc) override;

private:
    void advance_time(float dt);

    DestructionFunctionsRemovalTokens on_destroy_follower_node_;
    AdvanceTimes& advance_times_;
    Scene& scene_;
    const SelectedCameras& cameras_;
    FixedArray<float, 3> offset_;
    FixedArray<float, 3> grid_;
    DanglingPtr<SceneNode> follower_node_;
    TransformationMatrix<float, ScenePos, 3> transformation_matrix_;
    std::unique_ptr<EventReceiverDeletionToken> camera_changed_deletion_token_;
};

}
