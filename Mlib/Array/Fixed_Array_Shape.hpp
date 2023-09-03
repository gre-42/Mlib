#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <size_t... tsize>
class FixedArrayShape;

class ArrayShape;

namespace FasUtils {
    template <size_t... tsize_a, size_t... tsize_b>
    constexpr auto concatenated(FixedArrayShape<tsize_a...>, FixedArrayShape<tsize_b...>);
}

template <size_t... tsize>
class FixedArrayShape {
public:
    using A = FixedArrayShape;
    constexpr static auto erased_first();
    constexpr static auto erased_last();
    template <size_t... tsize_b>
    constexpr static auto concatenated(const FixedArrayShape<tsize_b...>& b) { return ::Mlib::FasUtils::concatenated(A(), b); }
    constexpr static auto last();
    constexpr static auto nelements();
    constexpr static auto ndim();
    constexpr static auto rows_as_1D();
    constexpr static auto columns_as_1D();
    template <size_t N>
    constexpr static size_t get();
};

namespace RUtils {
    //Strip last size_t in FixedArrayShape type
    template <size_t... I>
    struct rstrip;

    template <size_t I>
    struct rstrip<I> {
        using type = FixedArrayShape<>;
    };

    template<typename, typename>
    struct concat {};

    template<size_t... Is1, size_t... Is2>
    struct concat<FixedArrayShape<Is1...>, FixedArrayShape<Is2...>> {
        using type = FixedArrayShape<Is1..., Is2...>;
    };

    template <size_t I, size_t... Is>
    struct rstrip<I, Is...> {
        using type = typename concat<FixedArrayShape<I>, typename rstrip<Is...>::type>::type;
    };

    template <size_t... I>
    struct rget {};

    template <size_t I>
    struct rget<I> {
        static const size_t value = I;
    };

    template <size_t I, size_t... Is>
    struct rget<I, Is...> {
        static const size_t value = rget<Is...>::value;
    };
}

namespace FasUtils {
    template <size_t tsize_begin, size_t... tsize_end>
    constexpr auto erased_first(FixedArrayShape<tsize_begin, tsize_end...>) {
        return FixedArrayShape<tsize_end...>();
    }

    template <size_t... tsize>
    constexpr auto erased_last(FixedArrayShape<tsize...>) {
        return typename RUtils::rstrip<tsize...>::type();
    }

    template <size_t... tsize>
    constexpr auto last(FixedArrayShape<tsize...>) {
        return RUtils::rget<tsize...>::value;
    }

    template <size_t... tsize_a, size_t... tsize_b>
    constexpr auto concatenated(FixedArrayShape<tsize_a...>, FixedArrayShape<tsize_b...>) {
        return FixedArrayShape<tsize_a..., tsize_b...>();
    };

#ifdef _MSC_VER
    constexpr inline FixedArrayShape<1, 1> rows_as_1D(FixedArrayShape<>) {
        return FixedArrayShape<1, 1>();
    }

    template <size_t tsize0>
    constexpr FixedArrayShape<1, tsize0> rows_as_1D(FixedArrayShape<tsize0>) {
        return FixedArrayShape<1, tsize0>();
    }

    template <size_t tsize0, size_t tsize1>
    constexpr FixedArrayShape<tsize0, tsize1> rows_as_1D(FixedArrayShape<tsize0, tsize1>) {
        return FixedArrayShape<tsize0, tsize1>();
    }

    template <size_t tsize0, size_t tsize1, size_t tsize2>
    constexpr FixedArrayShape<tsize0 * tsize1, tsize2> rows_as_1D(FixedArrayShape<tsize0, tsize1, tsize2>) {
        return FixedArrayShape<tsize0 * tsize1, tsize2>();
    }
#endif

    template <size_t... tsize>
    constexpr auto rows_as_1D(FixedArrayShape<tsize...> a) {
        return FixedArrayShape<decltype(a.erased_last())::nelements()>().concatenated(FixedArrayShape<decltype(a)::last()>());
    }

#ifdef _MSC_VER
    constexpr inline FixedArrayShape<1, 1> columns_as_1D(FixedArrayShape<>) {
        return FixedArrayShape<1, 1>();
    }

    template <size_t tsize0>
    constexpr FixedArrayShape<tsize0, 1> columns_as_1D(FixedArrayShape<tsize0>) {
        return FixedArrayShape<tsize0, 1>();
    }

    template <size_t tsize0, size_t tsize1>
    constexpr FixedArrayShape<tsize0, tsize1> columns_as_1D(FixedArrayShape<tsize0, tsize1>) {
        return FixedArrayShape<tsize0, tsize1>();
    }

    template <size_t tsize0, size_t tsize1, size_t tsize2>
    constexpr FixedArrayShape<tsize0, tsize1 * tsize2> columns_as_1D(FixedArrayShape<tsize0, tsize1, tsize2>) {
        return FixedArrayShape<tsize0, tsize1 * tsize2>();
    }
#endif

    template <size_t tsize0, size_t... tsize>
    constexpr auto columns_as_1D(FixedArrayShape<tsize0, tsize...>) {
        constexpr FixedArrayShape<tsize...> a;
        return FixedArrayShape<tsize0>().concatenated(FixedArrayShape<decltype(a)::nelements()>());
    }

    template <size_t... tsize>
    inline void equals_(size_t* other, bool* result);

    template <>
    inline void equals_(size_t* other, bool* result) {}

    template <size_t tsize0, size_t... tsize>
    inline bool equals_(size_t* other, bool* result) {
        *result = (tsize0 == *other);
        return equals_<tsize...>(other + 1, result + 1);
    }

    template <size_t... tsize>
    inline void equals(size_t* other, size_t nelems, bool* result) {
        constexpr FixedArrayShape<tsize...> a;
        assert(nelems == nelements(a));
        return equals_<tsize...>(other, result);
    }

    constexpr size_t nelements(FixedArrayShape<>) {
        return 1;
    }

    template <size_t tsize0, size_t... tsize>
    constexpr size_t nelements(FixedArrayShape<tsize0, tsize...>) {
        constexpr FixedArrayShape<tsize...> a;
        return tsize0 * nelements(a);
    }

    constexpr size_t ndim(FixedArrayShape<>) {
        return 0;
    }

    template <size_t tsize0, size_t... tsize>
    constexpr size_t ndim(FixedArrayShape<tsize0, tsize...>) {
        constexpr FixedArrayShape<tsize...> a;
        return 1 + ndim(a);
    }
}

namespace ElemUtils {

    // From: https://stackoverflow.com/questions/20162903/template-parameter-packs-access-nth-type-and-nth-element
    template <size_t N, size_t... tsize>
    struct NthElement;

    template <size_t tsize0, size_t... tsize>
    struct NthElement<0, tsize0, tsize...> {
        static const size_t value = tsize0;
    };
    template <size_t N, size_t tsize0, size_t... tsize>
    struct NthElement<N, tsize0, tsize...> {
        static const size_t value = NthElement<N - 1, tsize...>::value;
    };

}

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::erased_first() { return ::Mlib::FasUtils::erased_first(A()); }

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::erased_last() { return ::Mlib::FasUtils::erased_last(A()); }

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::last() { return ::Mlib::FasUtils::last(A()); }

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::nelements() { return ::Mlib::FasUtils::nelements(A()); }

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::ndim() { return ::Mlib::FasUtils::ndim(A()); }

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::rows_as_1D() { return ::Mlib::FasUtils::rows_as_1D(A()); }

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::columns_as_1D() { return ::Mlib::FasUtils::columns_as_1D(A()); }

template <size_t... tsize>
template <size_t N>
constexpr size_t FixedArrayShape<tsize...>::get() { return ::Mlib::ElemUtils::NthElement<N, tsize...>::value; }

}
