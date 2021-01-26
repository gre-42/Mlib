#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

template <size_t... tsize>
class FixedArrayShape;

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
    constexpr auto concatenated(const FixedArrayShape<tsize_a...>*, const FixedArrayShape<tsize_b...>&) {
        return FixedArrayShape<tsize_a..., tsize_b...>();
    };

    template <class TData, size_t... tsize>
    constexpr auto make_shape(const FixedArray<TData, tsize...>*) {
        return FixedArrayShape<tsize...>();
    }

    template <class TData, size_t... tsize, size_t... tnew_size>
    FixedArray<TData, tnew_size...> reshape_fixed(const FixedArray<TData, tsize...>* a, const FixedArrayShape<tnew_size...>*) {
        return a.template reshaped<tnew_size...>();
    }

    template <size_t... tsize>
    constexpr auto rows_as_1D(const FixedArrayShape<tsize...>*) {
        constexpr static const FixedArrayShape<tsize...>* a = (const FixedArrayShape<tsize...>*)nullptr;
        return FixedArrayShape<a->erased_last().nelements()>().concatenated(FixedArrayShape<a->last()>());
    }

    template <size_t tsize0, size_t... tsize>
    constexpr auto columns_as_1D(const FixedArrayShape<tsize0, tsize...>*) {
        constexpr static const FixedArrayShape<tsize...>* a = (const FixedArrayShape<tsize...>*)nullptr;
        return FixedArrayShape<tsize0>().concatenated(FixedArrayShape<a->nelements()>());
    }

    constexpr auto nelements(const FixedArrayShape<>*) {
        return 1;
    }

    constexpr auto nelements(const FixedArrayShape<>&) {
        return 1;
    }

    template <size_t tsize0, size_t... tsize>
    constexpr auto nelements(const FixedArrayShape<tsize0, tsize...>*) {
        return tsize0 * nelements(FixedArrayShape<tsize...>());
    }

    template <size_t tsize0, size_t... tsize>
    constexpr auto nelements(const FixedArrayShape<tsize0, tsize...> &) {
        return tsize0 * nelements(FixedArrayShape<tsize...>());
    }
    }

template <size_t... tsize>
class FixedArrayShape {
public:
    constexpr static const FixedArrayShape<tsize...>* a = (const FixedArrayShape<tsize...>*)nullptr;
    constexpr auto erased_first() const { return ::Mlib::FasUtils::erased_first(a); }
    constexpr auto erased_last() const { return ::Mlib::FasUtils::erased_last(a); }
    template <size_t... tsize_b>
    constexpr auto concatenated(const FixedArrayShape<tsize_b...>& b) const { return ::Mlib::FasUtils::concatenated(a, b); }
    constexpr auto last() const { return ::Mlib::FasUtils::last(a); }
    constexpr auto nelements() const { return ::Mlib::FasUtils::nelements(a); }
    constexpr auto rows_as_1D() const { return ::Mlib::FasUtils::rows_as_1D(a); }
    constexpr auto columns_as_1D() const { return ::Mlib::FasUtils::columns_as_1D(a); }
};

}
