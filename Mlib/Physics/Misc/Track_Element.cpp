#include "Track_Element.hpp"
#include <Mlib/Io/Read_Number.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
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
    ostr << elapsed_seconds;
    for (const auto& t : transformations) {
        ostr << ' ' <<
            geographic_mapping.transform(t.position) << ' ' <<
            t.rotation;
    }
}

std::vector<double> TrackElement::to_vector(
    const TransformationMatrix<double, double, 3>& geographic_mapping) const
{
    std::vector<double> result(1 + 6 * transformations.size());
    result[0] = elapsed_seconds;
    for (size_t i = 0; i < transformations.size(); ++i) {
        auto pos = geographic_mapping.transform(transformations[i].position);
        result[1 + i * 6] = pos(0);
        result[2 + i * 6] = pos(1);
        result[3 + i * 6] = pos(2);
        result[4 + i * 6] = transformations[i].rotation(0);
        result[5 + i * 6] = transformations[i].rotation(1);
        result[6 + i * 6] = transformations[i].rotation(2);
    }
    return result;
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
        FixedArray<double, 3> pos = uninitialized;
        istr >>
            pos(0) >>
            pos(1) >>
            pos(2) >>
            t.rotation(0) >>
            t.rotation(1) >>
            t.rotation(2);
        t.position = inverse_geographic_mapping.transform(pos);
    }
    return result;
}

TrackElement TrackElement::from_vector(
    const std::vector<double>& data,
    const TransformationMatrix<double, double, 3>& inverse_geographic_mapping,
    size_t ntransformations)
{
    TrackElement result;
    if (data.size() != 1 + ntransformations * 6) {
        THROW_OR_ABORT("Unexpected track data vector size");
    }
    result.elapsed_seconds = (float)data[0];
    result.transformations.resize(ntransformations);
    for (auto&& [i, trafo] : enumerate(result.transformations)) {
        auto pos = FixedArray<double, 3>{
            data[1 + i * 6],
            data[2 + i * 6],
            data[3 + i * 6]};
        trafo.rotation = {
            (float)data[4 + i * 6],
            (float)data[5 + i * 6],
            (float)data[6 + i * 6]};
        trafo.position = inverse_geographic_mapping.transform(pos);
    }
    return result;
}

const OffsetAndTaitBryanAngles<float, ScenePos, 3>& TrackElement::transformation() const {
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
        auto qa = OffsetAndQuaternion<float, ScenePos>::from_tait_bryan_angles(a.transformations[i]);
        auto qb = OffsetAndQuaternion<float, ScenePos>::from_tait_bryan_angles(b.transformations[i]);
        auto qi = qa.slerp(qb, alpha);
        result.transformations[i].position = qi.t;
        result.transformations[i].rotation = qi.q.to_tait_bryan_angles();
    }
    return result;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const TrackElement& element) {
    ostr <<
        "elapsed [s]: " << element.elapsed_seconds;
    for (const auto& t : element.transformations) {
        ostr <<
            " pos [m]: " << t.position <<
            " rot [deg]: " << t.rotation / degrees;
    }
    return ostr;
}
