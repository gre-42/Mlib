#pragma once
#include <cstddef>

namespace Mlib {

template <class TDataX, class TDataY>
class Interp;
template <typename TData, size_t... tshape>
class FixedArray;

class IIlluminatable {
public:
    virtual void advance_illumination_time(float dt) = 0;
    virtual void illuminate(const Interp<float, FixedArray<float, 3>>& emmisive) = 0;
};

}
