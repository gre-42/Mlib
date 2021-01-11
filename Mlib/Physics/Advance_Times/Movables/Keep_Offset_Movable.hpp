#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <memory>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class Scene;

class KeepOffsetMovable: public DestructionObserver, public AbsoluteMovable, public AdvanceTime {
public:
    KeepOffsetMovable(
        AdvanceTimes& advance_times,
        Scene& scene,
        const std::string& follower_name,
        SceneNode* followed_node,
        AbsoluteMovable* followed,
        const FixedArray<float, 3>& offset);
    ~KeepOffsetMovable();
    virtual void advance_time(float dt) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float>& absolute_model_matrix) override;
    virtual TransformationMatrix<float> get_new_absolute_model_matrix() const override;
    virtual void notify_destroyed(void* obj) override;

private:
    AdvanceTimes& advance_times_;
    Scene& scene_;
    std::string follower_name_;
    SceneNode* followed_node_;
    AbsoluteMovable* followed_;
    FixedArray<float, 3> offset_;
    TransformationMatrix<float> transformation_matrix_;
};

}
