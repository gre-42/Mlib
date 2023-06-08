#include "Track_Element.hpp"
#include <Mlib/Io/Read_Number.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Quaternion.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Physics/Units.hpp>
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
        elapsed_seconds;
    for (const auto& t : transformations) {
        ostr << ' ' <<
            geographic_mapping.transform(t.position().casted<double>()) << ' ' <<
            t.rotation();
    }
}

TrackElement TrackElement::from_stream(
    std::istream& istr,
    const TransformationMatrix<double, double, 3>& inverse_geographic_mapping,
    size_t ntransformations)
{
    TrackElement result;
    istr >> ReadNum{result.elapsed_seconds};
    result.transformations.resize(ntransformations);
    for (auto& t : result.transformations) {
        FixedArray<double, 3> pos;
        istr >>
            pos(0) >>
            pos(1) >>
            pos(2) >>
            t.rotation(0) >>
            t.rotation(1) >>
            t.rotation(2);
        t.position() = inverse_geographic_mapping.transform(pos);
    }
    return result;
}

const OffsetAndTaitBryanAngles<float, double, 3>& TrackElement::transformation() const {
    if (transformations.empty()) {
        THROW_OR_ABORT("Track element is empty");
    }
    return transformations.front();
}

TrackElement Mlib::interpolated(const TrackElement& a, const TrackElement& b, float alpha) {
    if (a.transformations.size() != b.transformations.size()) {
        THROW_OR_ABORT("Mismatch in number of transformations");
    }
    TrackElement result;
    result.elapsed_seconds = (1 - alpha) * a.elapsed_seconds + alpha * b.elapsed_seconds;
    result.transformations.resize(a.transformations.size());
    for (size_t i = 0; i < a.transformations.size(); ++i) {
        auto qa = OffsetAndQuaternion<float, double>::from_tait_bryan_angles(a.transformations[i]);
        auto qb = OffsetAndQuaternion<float, double>::from_tait_bryan_angles(b.transformations[i]);
        auto qi = qa.slerp(qb, alpha);
        result.transformations[i].position() = qi.offset();
        result.transformations[i].rotation() = qi.quaternion().to_tait_bryan_angles();
    }
    return result;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const TrackElement& element) {
    ostr <<
        "elapsed [s]: " << element.elapsed_seconds;
    for (const auto& t : element.transformations) {
        ostr <<
            " pos [m]: " << t.position() <<
            " rot [deg]: " << t.rotation() / degrees;
    }
    return ostr;
}
