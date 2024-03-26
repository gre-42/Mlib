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

class AdvanceTimes;
class SceneNode;
class Scene;

class KeepOffsetFromMovable: public DestructionObserver<DanglingRef<SceneNode>>, public IAbsoluteMovable, public IAdvanceTime, public DanglingBaseClass {
public:
    KeepOffsetFromMovable(
        AdvanceTimes& advance_times,
        Scene& scene,
        const std::string& follower_name,
        DanglingRef<SceneNode> followed_node,
        IAbsoluteMovable& followed,
        const FixedArray<float, 3>& offset);
    ~KeepOffsetFromMovable();
    virtual void advance_time(float dt) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_absolute_model_matrix() const override;
    virtual void notify_destroyed(DanglingRef<SceneNode> destroyed_object) override;

private:
    AdvanceTimes& advance_times_;
    Scene& scene_;
    std::string follower_name_;
    DanglingPtr<SceneNode> followed_node_;
    IAbsoluteMovable* followed_;
    FixedArray<float, 3> offset_;
    TransformationMatrix<float, double, 3> transformation_matrix_;
};

}
