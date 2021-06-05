#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData, size_t ...tshape>
void assert_allclose(const FixedArray<TData, tshape...>& a, const FixedArray<TData, tshape...>& b, typename FloatType<TData>::value_type atol = 1e-6) {
    assert_allclose(a.to_array(), b.to_array(), atol);
}

template <class TData, size_t ...tshape>
void assert_allequal(const FixedArray<TData, tshape...>& a, const FixedArray<TData, tshape...>& b) {
    assert_allequal(a.to_array(), b.to_array(), atol);
}

}
