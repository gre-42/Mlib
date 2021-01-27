#pragma once
#include <Mlib/Template.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <size_t... tsize>
class FixedArrayShape;

namespace FasUtils {
    template <size_t... tsize_a, size_t... tsize_b>
    constexpr auto concatenated(const FixedArrayShape<tsize_a...>*, const FixedArrayShape<tsize_b...>*);
}

template <size_t... tsize>
class FixedArrayShape {
public:
    constexpr static const FixedArrayShape<tsize...>* a = nullptr;
    constexpr auto erased_first() const;
    constexpr auto erased_last() const;
    template <size_t... tsize_b>
    constexpr auto concatenated(const FixedArrayShape<tsize_b...>& b) const { return ::Mlib::FasUtils::concatenated(a, &b); }
    constexpr auto last() const;
    constexpr auto nelements() const;
    constexpr auto rows_as_1D() const;
    constexpr auto columns_as_1D() const;
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
    constexpr auto erased_first(const FixedArrayShape<tsize_begin, tsize_end...>*) {
        return FixedArrayShape<tsize_end...>();
    }

    template <size_t... tsize>
    constexpr auto erased_last(const FixedArrayShape<tsize...>*) {
        return typename RUtils::rstrip<tsize...>::type();
    }

    template <size_t... tsize>
    constexpr auto last(const FixedArrayShape<tsize...>*) {
        return RUtils::rget<tsize...>::value;
    }

    template <size_t... tsize_a, size_t... tsize_b>
    constexpr auto concatenated(const FixedArrayShape<tsize_a...>*, const FixedArrayShape<tsize_b...>*) {
        return FixedArrayShape<tsize_a..., tsize_b...>();
    };

#ifdef _MSC_VER
    constexpr inline FixedArrayShape<1, 1> rows_as_1D(const FixedArrayShape<>*) {
        return FixedArrayShape<1, 1>();
    }

    template <size_t tsize0>
    constexpr FixedArrayShape<1, tsize0> rows_as_1D(const FixedArrayShape<tsize0>*) {
        return FixedArrayShape<1, tsize0>();
    }

    template <size_t tsize0, size_t tsize1>
    constexpr FixedArrayShape<tsize0, tsize1> rows_as_1D(const FixedArrayShape<tsize0, tsize1>*) {
        return FixedArrayShape<tsize0, tsize1>();
    }

    template <size_t tsize0, size_t tsize1, size_t tsize2>
    constexpr FixedArrayShape<tsize0 * tsize1, tsize2> rows_as_1D(const FixedArrayShape<tsize0, tsize1, tsize2>*) {
        return FixedArrayShape<tsize0 * tsize1, tsize2>();
    }
#endif

    template <size_t... tsize>
    constexpr auto rows_as_1D(const FixedArrayShape<tsize...>*) {
        constexpr const FixedArrayShape<tsize...>* a = nullptr;
        return FixedArrayShape<a->erased_last().nelements()>().concatenated(FixedArrayShape<a->last()>());
    }

#ifdef _MSC_VER
    constexpr inline FixedArrayShape<1, 1> columns_as_1D(const FixedArrayShape<>*) {
        return FixedArrayShape<1, 1>();
    }

    template <size_t tsize0>
    constexpr FixedArrayShape<tsize0, 1> columns_as_1D(const FixedArrayShape<tsize0>*) {
        return FixedArrayShape<tsize0, 1>();
    }

    template <size_t tsize0, size_t tsize1>
    constexpr FixedArrayShape<tsize0, tsize1> columns_as_1D(const FixedArrayShape<tsize0, tsize1>*) {
        return FixedArrayShape<tsize0, tsize1>();
    }

    template <size_t tsize0, size_t tsize1, size_t tsize2>
    constexpr FixedArrayShape<tsize0, tsize1 * tsize2> columns_as_1D(const FixedArrayShape<tsize0, tsize1, tsize2>*) {
        return FixedArrayShape<tsize0, tsize1 * tsize2>();
    }
#endif

    template <size_t tsize0, size_t... tsize>
    constexpr auto columns_as_1D(const FixedArrayShape<tsize0, tsize...>*) {
        constexpr const FixedArrayShape<tsize...>* a = nullptr;
        return FixedArrayShape<tsize0>().concatenated(FixedArrayShape<a->nelements()>());
    }

    constexpr size_t nelements(const FixedArrayShape<>*) {
        return 1;
    }

    template <size_t tsize0, size_t... tsize>
    constexpr auto nelements(const FixedArrayShape<tsize0, tsize...>*) {
        constexpr const FixedArrayShape<tsize...>* a = nullptr;
        return tsize0 * nelements(a);
    }
}

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::erased_first() const { return ::Mlib::FasUtils::erased_first(a); }

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::erased_last() const { return ::Mlib::FasUtils::erased_last(a); }

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::last() const { return ::Mlib::FasUtils::last(a); }

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::nelements() const { return ::Mlib::FasUtils::nelements(a); }

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::rows_as_1D() const { return ::Mlib::FasUtils::rows_as_1D(a); }

template <size_t... tsize>
constexpr auto FixedArrayShape<tsize...>::columns_as_1D() const { return ::Mlib::FasUtils::columns_as_1D(a); }

}
