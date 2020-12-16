#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Array/Sparse_Array.hpp>

namespace Mlib {

template <class TData, size_t tndim>
struct PointsAndAdjacency {
    std::vector<FixedArray<TData, tndim>> points;
    SparseArrayCcs<TData> adjacency;
};

}
