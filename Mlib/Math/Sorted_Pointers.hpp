#pragma once
#include <Mlib/Iterator/Iterator.hpp>
#include <Mlib/Iterator/Non_Const_Sized_Iterable.hpp>
#include <Mlib/Iterator/Sized_Iterable_Factory.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <algorithm>
#include <ranges>
#include <vector>

namespace Mlib {

template <NonConstSizedIterable TNonConstSizedIterable, class TCompare = std::ranges::less>
[[nodiscard]] auto sorted_pointers(
    const TNonConstSizedIterable& iterable,
    const TCompare& compare = {})
{
    using T = std::remove_reference_t<decltype(*iterable.begin())>;
    std::vector<const T*> pointers;
    pointers.reserve(iterable.size());
    for (const auto& it : iterable) {
        pointers.push_back(&it);
    }
    std::sort(pointers.begin(), pointers.end(), [&compare](const T* a, const T* b){
        return compare(*a, *b);
    });
    return pointers;
}

template <Iterator TBegin, class TEnd, class TCompare = std::ranges::less>
[[nodiscard]] auto sorted_pointers(
    const TBegin& begin,
    const TEnd& end,
    const TCompare& compare = {})
{
    return sorted_pointers(
        SizedIterableFactory{
            begin,
            end,
            integral_cast<size_t>(end - begin)
        },
        compare);
}

}
