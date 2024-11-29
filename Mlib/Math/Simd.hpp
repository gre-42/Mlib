#include <std-simd/experimental/simd>
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template <class TData>
auto to_simd(const FixedArray<TData, 3>& a) {
    using Int = decltype(a(0).count);
    std::experimental::fixed_size_simd<Int, 3> result;
    result.copy_from((const Int*)a.flat_begin(), std::experimental::vector_aligned);
    return result;
}

template <typename Tp>
bool any(const std::experimental::simd_mask<Tp, std::experimental::simd_abi::fixed_size<3>>& a) {
    using namespace std::experimental;
    return a[0] || a[1] || a[2];
}

}
