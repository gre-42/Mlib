#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <stdexcept>

namespace Mlib {

class PointException: public std::runtime_error {
public:
    PointException(const FixedArray<float, 3>& point, const std::string& what)
    : std::runtime_error{what},
      point{point}
    {}
    FixedArray<float, 3> point;
};

}
