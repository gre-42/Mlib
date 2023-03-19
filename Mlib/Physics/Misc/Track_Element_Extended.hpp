#pragma once
#include <Mlib/Physics/Misc/Track_Element.hpp>

namespace Mlib {

enum class TrackElementInterpolationKey {
    ELAPSED_SECONDS,
    METERS_TO_START
};

struct TrackElementExtended {
    TrackElement element;
    double meters_to_start;
    static TrackElementExtended from_stream(
        const std::optional<TrackElementExtended>& predecessor,
        std::istream& istr,
        const TransformationMatrix<double, double, 3>& geographic_mapping);
    inline double progress(TrackElementInterpolationKey key) const {
        if (key == TrackElementInterpolationKey::ELAPSED_SECONDS) {
            if (std::isnan(element.elapsed_seconds)) {
                THROW_OR_ABORT("Elapsed seconds is NAN despite selected interpolation key");
            }
            return element.elapsed_seconds;
        }
        if (key == TrackElementInterpolationKey::METERS_TO_START) {
            return meters_to_start;
        }
        THROW_OR_ABORT("Unknown track element interpolation key");
    }
    inline const FixedArray<double, 3>& position() const {
        return element.position;
    }
    inline const FixedArray<float, 3>& rotation() const {
        return element.rotation;
    }
    inline void set_y_position(double value) {
        element.position(1) = value;
    }
};

TrackElementExtended interpolated(const TrackElementExtended& a, const TrackElementExtended& b, float alpha);

}
