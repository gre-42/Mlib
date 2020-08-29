#pragma once
#include <Mlib/Array/Array.hpp>

namespace Mlib {

Array<size_t> set_difference(const Array<size_t>& a, const Array<size_t>& b) {
    std::vector<size_t> av{a.flat_iterable().begin(), a.flat_iterable().end()};
    std::vector<size_t> bv{b.flat_iterable().begin(), b.flat_iterable().end()};
    Array<size_t> res(ArrayShape{av.size()});
    std::sort(av.begin(), av.end());
    std::sort(bv.begin(), bv.end());
    auto it = std::set_difference(av.begin(), av.end(), bv.begin(), bv.end(), res.flat_iterable().begin());
    res.reshape(it - res.flat_iterable().begin());
    return res;
}

}
