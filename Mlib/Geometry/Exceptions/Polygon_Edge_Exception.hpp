#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <sstream>
#include <stdexcept>

namespace Mlib {

template <class TData, size_t tndim>
class PolygonEdgeException : public std::runtime_error {
public:
    PolygonEdgeException (const FixedArray<TData, tndim, 3>& poly, size_t i0, size_t i1, const std::string& what)
        : std::runtime_error{ what }
        , poly{ poly }
        , i0{ i0 }
        , i1{ i1 }
    {}
    std::string str(const std::string& message, const TransformationMatrix<double, double, 3>* m) const {
        std::stringstream sstr;
        sstr.precision(15);
        sstr << message << " at edge " <<
            i0 << ", " <<
            i1 << " of polygon ";
        for (size_t i = 0; i < tndim; ++i) {
            if (i != 0) {
                sstr << " <-> ";
            }
            sstr << poly[i];
        }
        if (m != nullptr) {
            sstr <<
            " | ";
            for (size_t i = 0; i < tndim; ++i) {
                if (i != 0) {
                    sstr << " <-> ";
                }
                sstr << m->transform(poly[i].template casted<double>());
            }
        }
        sstr << ": " << what();
        return sstr.str();
    }
    FixedArray<TData, tndim, 3> poly;
    size_t i0;
    size_t i1;
};

}
