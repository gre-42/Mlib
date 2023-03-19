#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <iosfwd>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

struct TrackElement {
    float elapsed_seconds;
    FixedArray<double, 3> position;
    FixedArray<float, 3> rotation;
    void write_to_stream(std::ostream& ostr, const TransformationMatrix<double, double, 3>& geographic_mapping) const;
    static TrackElement from_stream(std::istream& istr, const TransformationMatrix<double, double, 3>& geographic_mapping);
};

TrackElement interpolated(const TrackElement& a, const TrackElement& b, float alpha);

}
