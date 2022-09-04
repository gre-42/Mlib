#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Reader.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <chrono>
#include <fstream>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class Focuses;

class RigidBodyPlayback: public DestructionObserver, public AdvanceTime, public AbsoluteMovable {
public:
    RigidBodyPlayback(
        const std::string& filename,
        AdvanceTimes& advance_times,
        const Focuses& focuses,
        const TransformationMatrix<double, double, 3>* geographic_mapping,
        float speedup);
    ~RigidBodyPlayback();
    virtual void advance_time(float dt) override;
    virtual void notify_destroyed(Object* obj) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_absolute_model_matrix() const override;
private:
    AdvanceTimes& advance_times_;
    const Focuses& focuses_;
    TransformationMatrix<float, double, 3> transformation_matrix_;
    float speedup_;
    TrackReader track_reader_;
};

}
