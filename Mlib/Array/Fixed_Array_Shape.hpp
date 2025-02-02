#pragma once
#include <algorithm>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <size_t... tsize>
class FixedArrayShape;

class ArrayShape;

namespace FasUtils {
    template <size_t... tsize_a, size_t... tsize_b>
    consteval auto concatenated(FixedArrayShape<tsize_a...>, FixedArrayShape<tsize_b...>);
}

template <size_t... tsize>
class FixedArrayShape {
public:
    using A = FixedArrayShape;
    consteval static auto erased_first();
    consteval static auto erased_last();
    template <size_t... tsize_b>
    consteval static auto concatenated(const FixedArrayShape<tsize_b...>& b) { return ::Mlib::FasUtils::concatenated(A(), b); }
    consteval static auto last();
    consteval static auto nelements();
    consteval static auto ndim();
    consteval static auto rows_as_1D();
    consteval static auto columns_as_1D();
    template <size_t axis>
    consteval static auto axis_as_3D();
    template <size_t axis>
    consteval static auto without_axis();
    template <size_t N>
    consteval static size_t get();
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
    consteval auto erased_first(FixedArrayShape<tsize_begin, tsize_end...>) {
        return FixedArrayShape<tsize_end...>();
    }

    template <size_t... tsize>
    consteval auto erased_last(FixedArrayShape<tsize...>) {
        return typename RUtils::rstrip<tsize...>::type();
    }

    template <size_t... tsize>
    consteval auto last(FixedArrayShape<tsize...>) {
        return RUtils::rget<tsize...>::value;
    }

    template <size_t... tsize_a, size_t... tsize_b>
    consteval auto concatenated(FixedArrayShape<tsize_a...>, FixedArrayShape<tsize_b...>) {
        return FixedArrayShape<tsize_a..., tsize_b...>();
    };

#ifdef _MSC_VER
    consteval inline FixedArrayShape<1, 1> rows_as_1D(FixedArrayShape<>) {
        return FixedArrayShape<1, 1>();
    }

    template <size_t tsize0>
    consteval FixedArrayShape<1, tsize0> rows_as_1D(FixedArrayShape<tsize0>) {
        return FixedArrayShape<1, tsize0>();
    }

    template <size_t tsize0, size_t tsize1>
    consteval FixedArrayShape<tsize0, tsize1> rows_as_1D(FixedArrayShape<tsize0, tsize1>) {
        return FixedArrayShape<tsize0, tsize1>();
    }

    template <size_t tsize0, size_t tsize1, size_t tsize2>
    consteval FixedArrayShape<tsize0 * tsize1, tsize2> rows_as_1D(FixedArrayShape<tsize0, tsize1, tsize2>) {
        return FixedArrayShape<tsize0 * tsize1, tsize2>();
    }
#endif

    template <size_t... tsize>
    consteval auto rows_as_1D(FixedArrayShape<tsize...> a) {
        return FixedArrayShape<decltype(a.erased_last())::nelements()>().concatenated(FixedArrayShape<decltype(a)::last()>());
    }

#ifdef _MSC_VER
    consteval inline FixedArrayShape<1, 1> columns_as_1D(FixedArrayShape<>) {
        return FixedArrayShape<1, 1>();
    }

    template <size_t tsize0>
    consteval FixedArrayShape<tsize0, 1> columns_as_1D(FixedArrayShape<tsize0>) {
        return FixedArrayShape<tsize0, 1>();
    }

    template <size_t tsize0, size_t tsize1>
    consteval FixedArrayShape<tsize0, tsize1> columns_as_1D(FixedArrayShape<tsize0, tsize1>) {
        return FixedArrayShape<tsize0, tsize1>();
    }

    template <size_t tsize0, size_t tsize1, size_t tsize2>
    consteval FixedArrayShape<tsize0, tsize1 * tsize2> columns_as_1D(FixedArrayShape<tsize0, tsize1, tsize2>) {
        return FixedArrayShape<tsize0, tsize1 * tsize2>();
    }
#endif

    template <size_t tsize0, size_t... tsize>
    consteval auto columns_as_1D(FixedArrayShape<tsize0, tsize...>) {
        constexpr FixedArrayShape<tsize...> a;
        return FixedArrayShape<tsize0>().concatenated(FixedArrayShape<decltype(a)::nelements()>());
    }

    template <size_t begin, size_t end>
    consteval auto sub_range(FixedArrayShape<>) {
        static_assert(end == 0);
        return FixedArrayShape<>();
    }
    
    template <size_t begin, size_t end, size_t tsize0, size_t... tsize>
    consteval auto sub_range(FixedArrayShape<tsize0, tsize...>) {
        if constexpr (end == 0) {
            return FixedArrayShape<>();
        } else if constexpr (begin > 0) {
            constexpr FixedArrayShape<tsize...> a;
            return sub_range<begin - 1, end - 1>(a);
        } else {
            constexpr FixedArrayShape<tsize...> a;
            constexpr auto left = FixedArrayShape<tsize0>();
            constexpr size_t iright = std::max<size_t>(1, begin) - 1;
            constexpr auto right = sub_range<iright, end - 1>(a);
            return concatenated(left, right);
        }
    }

    template <size_t begin, size_t end, size_t... tsize>
    consteval auto prod(FixedArrayShape<tsize...>) {
        constexpr FixedArrayShape<tsize...> a;
        return sub_range<begin, end>(a).nelements();
    }

    template <size_t axis, size_t... tsize>
    consteval auto axis_as_3D(FixedArrayShape<tsize...>) {
        constexpr FixedArrayShape<tsize...> a;
        constexpr size_t left = prod<0, axis>(a);
        constexpr size_t center = a.template get<axis>();
        constexpr size_t right = prod<axis + 1, a.ndim()>(a);
        return FixedArrayShape<left, center, right>();
    }

    template <size_t axis, size_t... tsize>
    consteval auto without_axis(FixedArrayShape<tsize...>) {
        constexpr FixedArrayShape<tsize...> a;
        constexpr auto left = sub_range<0, axis>(a);
        constexpr auto right = sub_range<axis + 1, a.ndim()>(a);
        return concatenated(left, right);
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

    consteval size_t nelements(FixedArrayShape<>) {
        return 1;
    }

    template <size_t tsize0, size_t... tsize>
    consteval size_t nelements(FixedArrayShape<tsize0, tsize...>) {
        constexpr FixedArrayShape<tsize...> a;
        return tsize0 * nelements(a);
    }

    consteval size_t ndim(FixedArrayShape<>) {
        return 0;
    }

    template <size_t tsize0, size_t... tsize>
    consteval size_t ndim(FixedArrayShape<tsize0, tsize...>) {
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
consteval auto FixedArrayShape<tsize...>::erased_first() { return ::Mlib::FasUtils::erased_first(A()); }

template <size_t... tsize>
consteval auto FixedArrayShape<tsize...>::erased_last() { return ::Mlib::FasUtils::erased_last(A()); }

template <size_t... tsize>
consteval auto FixedArrayShape<tsize...>::last() { return ::Mlib::FasUtils::last(A()); }

template <size_t... tsize>
consteval auto FixedArrayShape<tsize...>::nelements() { return ::Mlib::FasUtils::nelements(A()); }

template <size_t... tsize>
consteval auto FixedArrayShape<tsize...>::ndim() { return ::Mlib::FasUtils::ndim(A()); }

template <size_t... tsize>
consteval auto FixedArrayShape<tsize...>::rows_as_1D() { return ::Mlib::FasUtils::rows_as_1D(A()); }

template <size_t... tsize>
consteval auto FixedArrayShape<tsize...>::columns_as_1D() { return ::Mlib::FasUtils::columns_as_1D(A()); }

template <size_t... tsize>
template <size_t axis>
consteval auto FixedArrayShape<tsize...>::axis_as_3D() { return ::Mlib::FasUtils::axis_as_3D<axis>(A()); }

template <size_t... tsize>
template <size_t axis>
consteval auto FixedArrayShape<tsize...>::without_axis() { return ::Mlib::FasUtils::without_axis<axis>(A()); }

template <size_t... tsize>
template <size_t N>
consteval size_t FixedArrayShape<tsize...>::get() { return ::Mlib::ElemUtils::NthElement<N, tsize...>::value; }

}
