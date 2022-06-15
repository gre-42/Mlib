#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <stdexcept>

namespace Mlib {

template <class TPos, size_t tndim>
class PointException: public std::runtime_error {
public:
    PointException(const FixedArray<TPos, tndim>& point, const std::string& what)
    : std::runtime_error{what},
      point{point}
    {}
    FixedArray<TPos, tndim> point;
};

}
