#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <iosfwd>

namespace Mlib {

template <class TData, size_t n>
class TransformationMatrix;

struct TrackElement {
    float elapsed_time;
    FixedArray<float, 3> position;
    FixedArray<float, 3> rotation;
    static TrackElement nan();
    void write_to_stream(std::ostream& ostr, const TransformationMatrix<double, 3>& geographic_mapping) const;
    static TrackElement from_stream(std::istream& istr, const TransformationMatrix<double, 3>& geographic_mapping);
};

TrackElement interpolated(const TrackElement& a, const TrackElement& b, float alpha);

}
