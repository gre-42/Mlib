#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Reader.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <chrono>
#include <fstream>
#include <memory>
#include <vector>

namespace Mlib {

class AdvanceTimes;
class SceneNode;
class Focuses;

class ITrackElementSequence;
class RigidBodyPlayback;

class RigidBodySinglePlayback: public AbsoluteMovable {
    friend RigidBodyPlayback;
public:
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_absolute_model_matrix() const override;
private:
    TransformationMatrix<float, double, 3> transformation_matrix_;
};

class RigidBodyPlayback: public Object, public AdvanceTime {
public:
    RigidBodyPlayback(
        std::unique_ptr<ITrackElementSequence>&& sequence,
        AdvanceTimes& advance_times,
        const Focuses& focuses,
        const TransformationMatrix<double, double, 3>* geographic_mapping,
        float speedup,
        size_t ntransformations);
    ~RigidBodyPlayback();
    virtual void advance_time(float dt) override;
    AbsoluteMovable& get_playback_object(size_t i);
private:
    AdvanceTimes& advance_times_;
    const Focuses& focuses_;
    float speedup_;
    double progress_;
    TrackReader track_reader_;
    std::vector<RigidBodySinglePlayback> playback_objects_;
};

}
