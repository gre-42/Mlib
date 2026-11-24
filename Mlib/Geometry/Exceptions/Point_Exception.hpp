#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <sstream>
#include <stdexcept>

namespace p2t {

struct Point;

}

namespace Mlib {

template <class TPos, size_t tndim>
class PointException: public std::runtime_error {
public:
    PointException(const FixedArray<TPos, tndim>& point, const std::string& what)
        : std::runtime_error{what},
          point{point}
    {}
    PointException(const p2t::Point* point, const std::string& what)
        : std::runtime_error{ what }
        , point{ (TPos)((double*)point)[0], (TPos)((double*)point)[1] }
    {
        static_assert(tndim == 2);
    }
    std::string str(const std::string& message, const TransformationMatrix<double, double, 3>* m) const {
        std::stringstream sstr;
        sstr.precision(15);
        sstr << message << " at position " << point;
        if (m != nullptr) {
            sstr << " | " << m->transform(point.template casted<double>());
        }
        sstr << ": " << what();
        return sstr.str();
    }
    FixedArray<TPos, tndim> point;
};

template <class TPos, size_t ndim>
inline PointException<TPos, ndim> point_exception(const FixedArray<TPos, ndim>& a, const std::string& what) {
    return PointException<TPos, ndim>{a, what};
}

inline PointException<double, 2> point_exception(const p2t::Point* a, const std::string& what) {
    return PointException<double, 2>{a, what};
}

}
