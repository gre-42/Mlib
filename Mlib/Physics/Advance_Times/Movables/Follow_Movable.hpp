#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <Mlib/Signal/Exponential_Smoother.hpp>
#include <Mlib/Signal/Kalman_Filter.hpp>
#include <memory>

namespace Mlib {

class AdvanceTimes;
class SceneNode;

class FollowMovable: public DestructionObserver, public AbsoluteMovable, public AdvanceTime {
public:
    FollowMovable(
        AdvanceTimes& advance_times,
        SceneNode* followed_node,
        AbsoluteMovable* followed,
        float attachment_distance,
        const FixedArray<float, 3>& node_displacement,
        const FixedArray<float, 3>& look_at_displacement,
        float snappiness = 2,
        float y_adaptivity = 15,
        float y_snappiness = 0.05,
        float dt = 1.f/60,
        float dt_ref = 1.f/60);
    virtual void advance_time(float dt) override;
    virtual void set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix) override;
    virtual FixedArray<float, 4, 4> get_new_absolute_model_matrix() const override;
    virtual void notify_destroyed(void* obj) override;

private:
    AdvanceTimes& advance_times_;
    SceneNode* followed_node_;
    AbsoluteMovable* followed_;
    float attachment_distance_;
    FixedArray<float, 2> attachment_position_;
    FixedArray<float, 3> node_displacement_;
    FixedArray<float, 3> look_at_displacement_;
    FixedArray<float, 3> position_;
    FixedArray<float, 3, 3> rotation_;
    FixedArray<float, 3> dpos_old_;
    float snappiness_;
    float y_adaptivity_;
    float y_adapt_;
    float dt_dt_ref_;
    KalmanFilter<float> kalman_filter_;
    ExponentialSmoother<float> exponential_smoother_;
};

}
