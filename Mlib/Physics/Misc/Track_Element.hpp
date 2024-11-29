#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Math/Transformation/Tait_Bryan_Angles.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <iosfwd>
#include <vector>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TDir, class TPos, size_t n>
class OffsetAndTaitBryanAngles;

struct TrackElement {
    void write_to_stream(
        std::ostream& ostr,
        const TransformationMatrix<double, double, 3>& geographic_mapping) const;
    std::vector<double> to_vector(
        const TransformationMatrix<double, double, 3>& geographic_mapping) const;
    static TrackElement from_stream(
        std::istream& istr,
        const TransformationMatrix<double, double, 3>& geographic_mapping,
        size_t ntransformations);
    static TrackElement from_vector(
        const std::vector<double>& data,
        const TransformationMatrix<double, double, 3>& geographic_mapping,
        size_t ntransformations);
    const OffsetAndTaitBryanAngles<float, ScenePos, 3>& transformation() const;

    float elapsed_seconds;
    UVector<OffsetAndTaitBryanAngles<float, ScenePos, 3>> transformations;
};

TrackElement interpolated(const TrackElement& a, const TrackElement& b, float alpha);

std::ostream& operator << (std::ostream& ostr, const TrackElement& element);

}
