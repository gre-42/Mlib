#pragma once
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;

class ITrailExtender {
public:
    virtual ~ITrailExtender() = default;
    virtual void append_location(const TransformationMatrix<float, double, 3>& location) = 0;

};

}
