#include "Track_Element.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <chrono>
#include <istream>

using namespace Mlib;

TrackElement TrackElement::nan() {
    return TrackElement{NAN, fixed_nans<float, 3>(), fixed_nans<float, 3>()};
}

std::ostream& Mlib::operator << (std::ostream& ostr, const TrackElement& e) {
    ostr <<
        e.elapsed_time << ' ' <<
        e.position << ' ' <<
        e.rotation;
    return ostr;
}

std::istream& Mlib::operator >> (std::istream& istr, TrackElement& e) {
    istr >>
        e.elapsed_time >>
        e.position(0) >>
        e.position(1) >>
        e.position(2) >>
        e.rotation(0) >>
        e.rotation(1) >>
        e.rotation(2);
    return istr;
}

TrackElement Mlib::interpolated(const TrackElement& a, const TrackElement& b, float alpha) {
    return TrackElement{
        .elapsed_time = (1 - alpha) * a.elapsed_time + alpha * b.elapsed_time,
        .position = (1 - alpha) * a.position + alpha * b.position,
        .rotation = matrix_2_tait_bryan_angles(
            (1 - alpha) * tait_bryan_angles_2_matrix(a.rotation) +
            alpha * tait_bryan_angles_2_matrix(b.rotation))};
}
