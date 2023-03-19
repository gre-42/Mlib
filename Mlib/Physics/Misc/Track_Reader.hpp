#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Misc/Track_Element_Extended.hpp>
#include <istream>
#include <optional>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class TrackReader {
public:
    explicit TrackReader(
        const std::string& filename,
        size_t nlaps,
        const TransformationMatrix<double, double, 3>* inverse_geographic_mapping,
        TrackElementInterpolationKey interpolation_key);
    ~TrackReader();
    bool read(float dprogress);
    bool eof() const;
    inline const TrackElementExtended& track_element() const {
        return track_element_;
    }
    inline size_t frame_id() const {
        return frame_id_;
    }
    inline size_t lap_id() const {
        return lap_id_;
    }
private:
    std::unique_ptr<std::istream> ifstr_;
    std::string filename_;
    TrackElementExtended track_element_;
    size_t frame_id_;
    size_t lap_id_;
    size_t nlaps_remaining_;
    const TransformationMatrix<double, double, 3>* inverse_geographic_mapping_;
    TrackElementInterpolationKey interpolation_key_;
    double progress_;
    std::optional<TrackElementExtended> track_element0_;
    std::optional<TrackElementExtended> track_element1_;
};

}
