#include "Track_Element.hpp"
#include <Mlib/Io/Read_Number.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Quaternion.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <chrono>
#include <iomanip>
#include <istream>

using namespace Mlib;

void TrackElement::write_to_stream(
    std::ostream& ostr,
    const TransformationMatrix<double, double, 3>& geographic_mapping) const
{
    ostr << std::setprecision(18) << std::scientific;
    ostr <<
        elapsed_seconds << ' ' <<
        geographic_mapping.transform(position.casted<double>()) << ' ' <<
        rotation;
}

TrackElement TrackElement::from_stream(
    std::istream& istr,
    const TransformationMatrix<double, double, 3>& inverse_geographic_mapping)
{
    TrackElement result;
    FixedArray<double, 3> pos;
    istr >>
        ReadNum{result.elapsed_seconds} >>
        pos(0) >>
        pos(1) >>
        pos(2) >>
        result.rotation(0) >>
        result.rotation(1) >>
        result.rotation(2);
    result.position = inverse_geographic_mapping.transform(pos);
    return result;
}

TrackElement Mlib::interpolated(const TrackElement& a, const TrackElement& b, float alpha) {
    auto qa = OffsetAndQuaternion<float, double>(a.position, Quaternion<float>::from_tait_bryan_angles(a.rotation));
    auto qb = OffsetAndQuaternion<float, double>(b.position, Quaternion<float>::from_tait_bryan_angles(b.rotation));
    auto qi = qa.slerp(qb, alpha);
    return TrackElement{
        .elapsed_seconds = (1 - alpha) * a.elapsed_seconds + alpha * b.elapsed_seconds,
        .position = qi.offset(),
        .rotation = qi.quaternion().to_tait_bryan_angles()};
}
