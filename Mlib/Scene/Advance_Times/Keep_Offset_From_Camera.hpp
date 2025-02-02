#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
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

class KeepOffsetFromCamera: public DestructionObserver<SceneNode&>, public IAbsoluteMovable, public IAdvanceTime, public virtual DanglingBaseClass {
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
    virtual void notify_destroyed(SceneNode& destroyed_object) override;

private:
    void advance_time(float dt);

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
