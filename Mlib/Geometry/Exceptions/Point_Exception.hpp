#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <sstream>
#include <stdexcept>

namespace Mlib {

template <class TPos, size_t tndim>
class PointException: public std::runtime_error {
public:
    PointException(const FixedArray<TPos, tndim>& point, const std::string& what)
        : std::runtime_error{what},
          point{point}
    {}
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

}
