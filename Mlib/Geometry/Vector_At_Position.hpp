#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <iosfwd>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

template <class TDir, class TPos, size_t tsize>
struct VectorAtPosition {
    FixedArray<TDir, tsize> vector;
    FixedArray<TPos, tsize> position;
    VectorAtPosition transformed(const TransformationMatrix<TDir, TPos, tsize>& m) const {
        return VectorAtPosition{
            .vector = m.rotate(vector),
            .position = m.transform(position)};
    }
};

template <class TDir, class TPos, size_t tsize>
std::ostream& operator << (std::ostream& ostr, const VectorAtPosition<TDir, TPos, tsize>& vp) {
    ostr << "v: " << vp.vector << ", p: " << vp.position;
    return ostr;
}

}
