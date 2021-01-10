#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Reader.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <chrono>
#include <fstream>

namespace Mlib {

class AdvanceTimes;
class SceneNode;

class RigidBodyPlayback: public DestructionObserver, public AdvanceTime, public AbsoluteMovable {
public:
    RigidBodyPlayback(
        const std::string& filename,
        AdvanceTimes& advance_times,
        const std::list<Focus>& focus,
        float speed);
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(void* obj) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float>& absolute_model_matrix) override;
    virtual TransformationMatrix<float> get_new_absolute_model_matrix() const override;
private:
    AdvanceTimes& advance_times_;
    const std::list<Focus>& focus_;
    FixedArray<float, 3> position_;
    FixedArray<float, 3, 3> rotation_;
    TrackReader track_reader_;
};

}
