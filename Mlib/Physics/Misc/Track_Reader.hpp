#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Misc/Track_Element_Extended.hpp>
#include <optional>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class ITrackElementSequence;
enum class TrackReaderInterpolationMode {
    NEAREST_NEIGHBOR,
    LINEAR
};

class TrackReader {
public:
    explicit TrackReader(
        std::unique_ptr<ITrackElementSequence>&& sequence,
        size_t nframes,
        size_t nlaps,
        const TransformationMatrix<double, double, 3>* inverse_geographic_mapping,
        TrackElementInterpolationKey interpolation_key,
        TrackReaderInterpolationMode interpolation_mode,
        size_t ntransformations);
    ~TrackReader();
    bool read(double& progress);
    bool finished() const;
    inline const TrackElementExtended& track_element() const {
        return track_element_;
    }
    inline size_t frame_id() const {
        return frame_id_;
    }
    inline size_t lap_id() const {
        return lap_id_;
    }
    inline double progress() const {
        if (!has_value()) {
            THROW_OR_ABORT("TrackReader::progress called on uninitialized object");
        }
        return track_element_.progress(interpolation_key_);
    }
    inline bool has_value() const {
        return track_element0_.has_value() && track_element1_.has_value();
    }
private:
    std::unique_ptr<ITrackElementSequence> sequence_;
    TrackElementExtended track_element_;
    size_t frame_id_;
    size_t lap_id_;
    size_t nframes_remaining_;
    size_t nlaps_remaining_;
    const TransformationMatrix<double, double, 3>* inverse_geographic_mapping_;
    TrackElementInterpolationKey interpolation_key_;
    TrackReaderInterpolationMode interpolation_mode_;
    size_t ntransformations_;
    std::optional<TrackElementExtended> track_element0_;
    std::optional<TrackElementExtended> track_element1_;
};

}
