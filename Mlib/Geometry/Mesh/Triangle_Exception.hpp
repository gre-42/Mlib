#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <stdexcept>

namespace Mlib {

    class TriangleException : public std::runtime_error {
    public:
        TriangleException(const FixedArray<float, 2>& a, const FixedArray<float, 2>& b, const FixedArray<float, 2>& c, const std::string& what)
            : std::runtime_error{ what },
            a{ FixedArray<float, 3>{a(0), a(1), 0.f} },
            b{ FixedArray<float, 3>{b(0), b(1), 0.f} },
            c{ FixedArray<float, 3>{c(0), c(1), 0.f} }
        {}
        TriangleException(const FixedArray<float, 3>& a, const FixedArray<float, 3>& b, const FixedArray<float, 3>& c, const std::string& what)
            : std::runtime_error{ what },
            a{ a },
            b{ b },
            c{ c }
        {}
        FixedArray<float, 3> a;
        FixedArray<float, 3> b;
        FixedArray<float, 3> c;
    };

}
