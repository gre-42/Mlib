#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <memory>

namespace Mlib {

template <class T>
class DanglingRef;
class AdvanceTimes;
class SceneNode;
class Scene;
class SelectedCameras;

class KeepOffsetFromCamera: public DestructionObserver<DanglingRef<const SceneNode>>, public AbsoluteMovable, public AdvanceTime {
public:
    KeepOffsetFromCamera(
        AdvanceTimes& advance_times,
        Scene& scene,
        const SelectedCameras& cameras,
        const FixedArray<float, 3>& offset,
        const FixedArray<float, 3>& grid);
    ~KeepOffsetFromCamera();
    virtual void advance_time(float dt) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_absolute_model_matrix() const override;
    virtual void notify_destroyed(DanglingRef<const SceneNode> destroyed_object) override;

private:
    AdvanceTimes& advance_times_;
    Scene& scene_;
    const SelectedCameras& cameras_;
    FixedArray<float, 3> offset_;
    FixedArray<float, 3> grid_;
    TransformationMatrix<float, double, 3> transformation_matrix_;
};

}
