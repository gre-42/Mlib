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
    static TrackElementExtended create(
        const std::optional<TrackElementExtended>& predecessor,
        const TrackElement& track_element);
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
    const OffsetAndTaitBryanAngles<float, ScenePos, 3>& transformation() const;
    void set_y_position(ScenePos value);
};

TrackElementExtended interpolated(const TrackElementExtended& a, const TrackElementExtended& b, float alpha);

std::ostream& operator << (std::ostream& ostr, const TrackElementExtended& element);

}
