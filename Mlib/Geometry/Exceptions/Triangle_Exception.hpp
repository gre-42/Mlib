#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <sstream>
#include <stdexcept>

namespace Mlib {

template <class TData>
class TriangleException : public std::runtime_error {
public:
    TriangleException(const FixedArray<TData, 2>& a, const FixedArray<TData, 2>& b, const FixedArray<TData, 2>& c, const std::string& what)
        : std::runtime_error{ what },
        a{ FixedArray<TData, 3>{a(0), a(1), TData(0)} },
        b{ FixedArray<TData, 3>{b(0), b(1), TData(0)} },
        c{ FixedArray<TData, 3>{c(0), c(1), TData(0)} }
    {}
    TriangleException(const FixedArray<TData, 3>& a, const FixedArray<TData, 3>& b, const FixedArray<TData, 3>& c, const std::string& what)
        : std::runtime_error{ what },
        a{ a },
        b{ b },
        c{ c }
    {}
    std::string str(const std::string& message, const TransformationMatrix<double, double, 3>* m) const {
        std::stringstream sstr;
        sstr.precision(15);
        sstr << message << " at triangle " <<
            a <<
            " <-> " <<
            b <<
            " <-> " <<
            c;
        if (m != nullptr) {
            sstr <<
                " | " <<
                m->transform(a.template casted<double>()) <<
                " <-> " <<
                m->transform(b.template casted<double>()) <<
                " <-> " <<
                m->transform(c.template casted<double>());
        }
        sstr << ": " << what();
        return sstr.str();
    }
    FixedArray<TData, 3> a;
    FixedArray<TData, 3> b;
    FixedArray<TData, 3> c;
};

}
