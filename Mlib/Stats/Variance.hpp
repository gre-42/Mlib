#pragma once
#include <Mlib/Math/Math.hpp>

namespace Mlib {

template <class TData>
TData var(const Array<TData>& a) {
    return sum(squared(a - mean(a))) / (a.nelements() - 1);
}

template <class TData>
TData nanvar(const Array<TData>& a) {
    return var(a[!Mlib::isnan(a)]);
}

template <class TData>
TData stddev(const Array<TData>& a) {
    return std::sqrt(var(a));
}

template <class TData>
TData nanstddev(const Array<TData>& a) {
    return stddev(a[!Mlib::isnan(a)]);
}

template <class TData>
Array<TData> standardize(const Array<TData>& a) {
    return (a - mean(a)) / stddev(a);
}

}
