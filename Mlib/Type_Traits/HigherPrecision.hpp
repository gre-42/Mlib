#pragma once

namespace Mlib {

template<class T>
struct HigherPrecision;

template<>
struct HigherPrecision<double> {
    using value_type = long double;
};

template<>
struct HigherPrecision<float> {
    using value_type = double;
};

}
