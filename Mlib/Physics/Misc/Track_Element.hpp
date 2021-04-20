#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <iosfwd>

namespace Mlib {

struct TrackElement {
    float elapsed_time;
    FixedArray<float, 3> position;
    FixedArray<float, 3> rotation;
    static TrackElement nan();
};

std::ostream& operator << (std::ostream& ostr, const TrackElement& e);

std::istream& operator >> (std::istream& istr, TrackElement& e);

TrackElement interpolated(const TrackElement& a, const TrackElement& b, float alpha);

}
