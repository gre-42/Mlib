#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IRelative_Movable.hpp>
#include <memory>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class Scene;

class CopyRotation: public DestructionObserver<DanglingRef<const SceneNode>>, public IRelativeMovable, public AdvanceTime {
public:
    CopyRotation(
        AdvanceTimes& advance_times,
        DanglingRef<SceneNode> from);
    ~CopyRotation();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, double, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_relative_model_matrix() const override;
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(DanglingRef<const SceneNode> destroyed_object) override;
private:
    AdvanceTimes& advance_times_;
    DanglingPtr<SceneNode> from_;
    TransformationMatrix<float, double, 3> transformation_matrix_;
};

}
