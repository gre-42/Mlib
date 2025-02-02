#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

template <class TData, size_t tshape0, size_t ...tshape>
void assert_allclose(const FixedArray<TData, tshape0, tshape...>& a, const FixedArray<TData, tshape0, tshape...>& b, typename FloatType<TData>::value_type atol = (typename FloatType<TData>::value_type)1e-6) {
    for (size_t i = 0; i < tshape0; ++i) {
        assert_allclose(a[i], b[i], atol);
    }
}

template <class TData>
void assert_allclose(const FixedArray<TData>& a, const FixedArray<TData>& b, typename FloatType<TData>::value_type atol = (typename FloatType<TData>::value_type)1e-6) {
    if (!isclose(a(), b(), atol)) {
        std::stringstream sstr;
        sstr << "Numbers not close (atol=" << atol << "):" << a() << ", " << b();
        THROW_OR_ABORT(sstr.str());
    }
}

template <class TData, size_t tshape0, size_t... tshape>
void assert_allequal(const FixedArray<TData, tshape0, tshape...>& a, const FixedArray<TData, tshape0, tshape...>& b) {
    for (size_t i = 0; i < tshape0; ++i) {
        assert_allequal(a[i], b[i]);
    }
}

template <class TData>
void assert_allequal(const FixedArray<TData>& a, const FixedArray<TData>& b) {
    if (!(a() == b()) && !(scalar_isnan(a()) && scalar_isnan(b()))) {
        THROW_OR_ABORT("Numbers not identical: " + std::to_string(a()) + ", " + std::to_string(b()));
    }
}

}
