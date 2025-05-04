#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <memory>

namespace Mlib {

class AdvanceTimes;
class Scene;
class SceneNode;

class LookAtMovable: public DestructionObserver<SceneNode&>, public IAbsoluteMovable, public IAdvanceTime, public virtual DanglingBaseClass {
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
    virtual void notify_destroyed(SceneNode& destroyed_object) override;

private:
    AdvanceTimes& advance_times_;
    Scene& scene_;
    VariableAndHash<std::string> follower_name_;
    DanglingPtr<SceneNode> follower_node_;
    DanglingPtr<SceneNode> followed_node_;
    IAbsoluteMovable* followed_;
    TransformationMatrix<float, ScenePos, 3> transformation_matrix_;
};

}
