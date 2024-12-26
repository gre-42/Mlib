#pragma once
#include <Mlib/Math/Fixed_Point_Number.hpp>

namespace Mlib {

template <class TData> struct FloatType { typedef typename TData::value_type value_type; };
template <> struct FloatType<float> { typedef float value_type; };
template <> struct FloatType<double> { typedef double value_type; };

}
