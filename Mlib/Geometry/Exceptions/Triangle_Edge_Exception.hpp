#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <sstream>
#include <stdexcept>

namespace Mlib {

template <class TData>
class TriangleEdgeException : public std::runtime_error {
public:
    TriangleEdgeException(const FixedArray<TData, 3>& a, const FixedArray<TData, 3>& b, const FixedArray<TData, 3>& c, size_t i0, size_t i1, const std::string& what)
        : std::runtime_error{ what },
        a{ a },
        b{ b },
        c{ c },
        i0{ i0 },
        i1{ i1 }
    {}
    std::string str(const std::string& message, const TransformationMatrix<double, double, 3>* m) const {
        std::stringstream sstr;
        sstr.precision(15);
        sstr << message << " at edge " <<
            i0 << ", " <<
            i1 << " of triangle " <<
            a <<
            " <-> " <<
            b <<
            " <-> " <<
            c;
        if (m != nullptr) {
            sstr <<
            " | " <<
            m->transform(a TEMPLATEV casted<double>()) <<
            " <-> " <<
            m->transform(b TEMPLATEV casted<double>()) <<
            " <-> " <<
            m->transform(c TEMPLATEV casted<double>());
        }
        sstr << ": " << what();
        return sstr.str();
    }
    FixedArray<TData, 3> a;
    FixedArray<TData, 3> b;
    FixedArray<TData, 3> c;
    size_t i0;
    size_t i1;
};

}
