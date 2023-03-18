#include "Track_Element_Extended.hpp"

using namespace Mlib;

TrackElementExtended TrackElementExtended::nan() {
    return {
        .element = TrackElement::nan(),
        .meters_to_start = NAN
    };
}

TrackElementExtended TrackElementExtended::from_stream(
    const TrackElementExtended& predecessor,
    std::istream& istr,
    const TransformationMatrix<double, double, 3>& geographic_mapping)
{
    TrackElementExtended result;
    result.element = TrackElement::from_stream(istr, geographic_mapping);
    if (std::isnan(predecessor.element.elapsed_seconds)) {
        result.meters_to_start = 0.;
    } else {
        auto ds = std::sqrt(sum(squared(predecessor.element.position - result.element.position)));
        result.meters_to_start = predecessor.meters_to_start + ds;
    }
    return result;
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
