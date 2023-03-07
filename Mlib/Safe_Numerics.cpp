#include "Safe_Numerics.hpp"

using namespace Mlib;

template <>
const double SafeLimits<double>::zero_threshold = 1e-14;

template <>
const double SafeLimits<double>::lower_threshold = 1e-13;

template <>
const double SafeLimits<double>::upper_threshold = 1e8;

template <>
const float SafeLimits<float>::zero_threshold = float(1e-14);

template <>
const float SafeLimits<float>::lower_threshold = float(1e-13);

template <>
const float SafeLimits<float>::upper_threshold = float(1e8);
