#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IRelative_Movable.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <memory>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class Scene;

class CopyRotation: public DestructionObserver<SceneNode&>, public IRelativeMovable, public IAdvanceTime, public virtual DanglingBaseClass {
public:
    explicit CopyRotation(DanglingRef<SceneNode> from);
    ~CopyRotation();
    virtual void set_initial_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) override;
    virtual void set_updated_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, ScenePos, 3> get_new_relative_model_matrix() const override;
    virtual void advance_time(float dt, const StaticWorld& world) override;
    virtual void notify_destroyed(SceneNode& destroyed_object) override;
private:
    DanglingPtr<SceneNode> from_;
    TransformationMatrix<float, ScenePos, 3> transformation_matrix_;
};

}
