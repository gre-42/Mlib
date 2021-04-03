#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <stdexcept>

namespace p2t {

struct Point;

}

namespace Mlib {

class EdgeException: public std::runtime_error {
public:
    EdgeException(const FixedArray<float, 3>& a, const FixedArray<float, 3>& b, const std::string& what)
    : std::runtime_error{what},
      a{a},
      b{b}
    {}
    EdgeException(const FixedArray<float, 2>& a, const FixedArray<float, 2>& b, const std::string& what)
    : std::runtime_error{what},
      a{a(0), a(1), 0.f},
      b{b(0), b(1), 0.f}
    {}
    EdgeException(const p2t::Point* a, const p2t::Point* b, const std::string& what)
    : std::runtime_error{what},
      a{(float)((double*)a)[0], (float)((double*)a)[1], 0.f},
      b{(float)((double*)b)[0], (float)((double*)b)[1], 0.f}
    {}
    FixedArray<float, 3> a;
    FixedArray<float, 3> b;
};

}
