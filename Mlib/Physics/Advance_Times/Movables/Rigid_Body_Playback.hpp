#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Physics/Misc/Track_Reader.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <chrono>
#include <fstream>
#include <memory>
#include <vector>

namespace Mlib {

class SceneNode;
class Focuses;

class ITrackElementSequence;
class RigidBodyPlayback;

class RigidBodySinglePlayback: public IAbsoluteMovable, public virtual DanglingBaseClass {
    friend RigidBodyPlayback;
public:
    RigidBodySinglePlayback();
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, ScenePos, 3> get_new_absolute_model_matrix() const override;
private:
    TransformationMatrix<float, ScenePos, 3> transformation_matrix_;
};

class RigidBodyPlayback: public IAdvanceTime, public virtual DanglingBaseClass {
public:
    RigidBodyPlayback(
        std::unique_ptr<ITrackElementSequence>&& sequence,
        const Focuses& focuses,
        const TransformationMatrix<double, double, 3>* geographic_mapping,
        float speedup,
        size_t ntransformations);
    ~RigidBodyPlayback();
    virtual void advance_time(float dt, const StaticWorld& world) override;
    DanglingBaseClassRef<IAbsoluteMovable> get_playback_object(size_t i);
private:
    const Focuses& focuses_;
    float speedup_;
    double progress_;
    TrackReader track_reader_;
    std::vector<std::unique_ptr<RigidBodySinglePlayback>> playback_objects_;
};

}
