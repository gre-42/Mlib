#include "Rigid_Body_Playback.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

RigidBodyPlayback::RigidBodyPlayback(
    std::unique_ptr<ITrackElementSequence>&& sequence,
    const Focuses& focuses,
    const TransformationMatrix<double, double, 3>* geographic_mapping,
    float speedup,
    size_t ntransformations)
    : focuses_{focuses}
    , speedup_{speedup}
    , progress_{0.}
    , track_reader_{
        std::move(sequence),
        0, // nframes
        1, // nlaps
        geographic_mapping,
        TrackElementInterpolationKey::ELAPSED_SECONDS,
        TrackReaderInterpolationMode::LINEAR,
        ntransformations
    }
{
    playback_objects_.resize(ntransformations);
    for (auto& o : playback_objects_) {
        o = std::make_unique<RigidBodySinglePlayback>();
    }
}

RigidBodyPlayback::~RigidBodyPlayback() {
    on_destroy.clear();
}

void RigidBodyPlayback::advance_time(float dt, const StaticWorld& world) {
    {
        std::shared_lock lock{focuses_.mutex};
        if (focuses_.countdown_active()) {
            return;
        }
    }
    if (track_reader_.read(progress_)) {
        progress_ += dt / seconds * speedup_;
        const auto& t = track_reader_.track_element().element.transformations;
        if (t.size() != playback_objects_.size()) {
            THROW_OR_ABORT("Conflicting playback sizees");
        }
        for (size_t i = 0; i < t.size(); ++i) {
            playback_objects_[i]->transformation_matrix_ = t[i].to_matrix();
        }
    }
}

DanglingBaseClassRef<IAbsoluteMovable> RigidBodyPlayback::get_playback_object(size_t i) {
    if (i >= playback_objects_.size()) {
        THROW_OR_ABORT("Playback-object index out of bounds");
    }
    return { *playback_objects_[i], CURRENT_SOURCE_LOCATION };
}

RigidBodySinglePlayback::RigidBodySinglePlayback()
    : transformation_matrix_{ fixed_nans<float, 3, 3>(), fixed_nans<ScenePos, 3>() }
{}

void RigidBodySinglePlayback::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) {
    transformation_matrix_ = absolute_model_matrix;
}

TransformationMatrix<float, ScenePos, 3> RigidBodySinglePlayback::get_new_absolute_model_matrix() const {
    return transformation_matrix_;
}
