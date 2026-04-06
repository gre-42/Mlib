#pragma once
#include <Mlib/Math/Sorted_Pointers.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <algorithm>
#include <ranges>
#include <vector>

namespace Mlib {

template <class TBegin, class TEnd, class TCompare = std::ranges::less, class TProjection = std::identity>
[[nodiscard]] auto sorted_deferred(
    const TBegin& begin,
    const TEnd& end,
    const TCompare& compare = {},
    const TProjection& project = {})
{
    auto pointers = sorted_pointers(begin, end, compare);
    std::vector<std::remove_reference_t<decltype(project(*begin))>> result;
    result.reserve(integral_cast<size_t>(end - begin));
    for (const auto* p : pointers) {
        result.push_back(project(*p));
    }
    return result;
}

}
