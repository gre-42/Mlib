#pragma once
#include <chrono>
#include <cstddef>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TData, size_t n>
struct FixedScaledUnitVector;

struct StaticWorld {
    const TransformationMatrix<double, double, 3>* geographic_mapping;
    const TransformationMatrix<double, double, 3>* inverse_geographic_mapping;
    const FixedScaledUnitVector<float, 3>* gravity;
    const FixedScaledUnitVector<float, 3>* wind;
    std::chrono::steady_clock::time_point time;
};

}
