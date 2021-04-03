#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <stdexcept>

namespace Mlib {

class EdgeException: public std::runtime_error {
public:
    EdgeException(const FixedArray<float, 3>& a, const FixedArray<float, 3>& b, const std::string& what)
    : std::runtime_error{what},
      a{a},
      b{b}
    {}
    FixedArray<float, 3> a;
    FixedArray<float, 3> b;
};

}
