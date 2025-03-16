#include "Track_Element_Extended.hpp"
#include <Mlib/Math/Transformation/Tait_Bryan_Angles.hpp>

using namespace Mlib;

TrackElementExtended TrackElementExtended::create(
    const std::optional<TrackElementExtended>& predecessor,
    const TrackElement& track_element)
{
    TrackElementExtended result;
    result.element = track_element;
    if (!predecessor.has_value()) {
        result.meters_to_start = 0.;
    } else {
        auto ds = std::sqrt(sum(squared(
            predecessor->transformation().position -
            result.transformation().position)));
        result.meters_to_start = predecessor->meters_to_start + ds;
    }
    return result;
}

const OffsetAndTaitBryanAngles<float, ScenePos, 3>& TrackElementExtended::transformation() const {
    return element.transformation();
}

void TrackElementExtended::set_y_position(ScenePos value) {
    if (element.transformations.empty()) {
        THROW_OR_ABORT("Extended track element is empty");
    }
    auto diff = value - element.transformations.front().position(1);
    for (auto& t : element.transformations) {
        t.position(1) += diff;
    }

}

TrackElementExtended Mlib::interpolated(
    const TrackElementExtended& a,
    const TrackElementExtended& b,
    float alpha)
{
    return {
        .element = interpolated(a.element, b.element, alpha),
        .meters_to_start = (1 - alpha) * a.meters_to_start + alpha * b.meters_to_start
    };
}

std::ostream& Mlib::operator << (std::ostream& ostr, const TrackElementExtended& element) {
    ostr << element.element << " meters_to_start: " << element.meters_to_start;
    return ostr;
}
