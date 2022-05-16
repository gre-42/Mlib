#include "Track_Element.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <chrono>
#include <istream>

using namespace Mlib;

TrackElement TrackElement::nan() {
    return TrackElement{NAN, fixed_nans<float, 3>(), fixed_nans<float, 3>()};
}

void TrackElement::write_to_stream(
    std::ostream& ostr,
    const TransformationMatrix<double, 3>& geographic_mapping) const
{
    ostr <<
        elapsed_time << ' ' <<
        geographic_mapping.transform(position.casted<double>()) << ' ' <<
        rotation;
}

TrackElement TrackElement::from_stream(
    std::istream& istr,
    const TransformationMatrix<double, 3>& inverse_geographic_mapping)
{
    TrackElement result;
    FixedArray<double, 3> pos;
    istr >>
        result.elapsed_time >>
        pos(0) >>
        pos(1) >>
        pos(2) >>
        result.rotation(0) >>
        result.rotation(1) >>
        result.rotation(2);
    result.position = inverse_geographic_mapping.transform(pos).casted<float>();
    return result;
}

TrackElement Mlib::interpolated(const TrackElement& a, const TrackElement& b, float alpha) {
    return TrackElement{
        .elapsed_time = (1 - alpha) * a.elapsed_time + alpha * b.elapsed_time,
        .position = (1 - alpha) * a.position + alpha * b.position,
        .rotation = matrix_2_tait_bryan_angles(
            (1 - alpha) * tait_bryan_angles_2_matrix(a.rotation) +
            alpha * tait_bryan_angles_2_matrix(b.rotation))};
}
