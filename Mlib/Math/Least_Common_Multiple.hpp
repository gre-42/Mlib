#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <algorithm>
#include <type_traits>
#include <vector>

namespace Mlib {

template <class TIterable>
auto least_common_multiple(
    const TIterable& begin,
    const TIterable& end,
    const std::remove_reference_t<decltype(*begin)>& tolerance,
    size_t max_iterations)
{
    std::vector<std::remove_reference_t<decltype(*begin)>> data0(begin, end);
    std::vector<std::remove_reference_t<decltype(*begin)>> dataI(begin, end);
    if (data0.empty()) {
        THROW_OR_ABORT("least_common_multiple received empty sequence");
    }
    for (size_t iteration = 0; iteration < max_iterations; ++iteration) {
        if (std::all_of(
            dataI.begin() + 1,
            dataI.end(),
            [&dataI, &tolerance](const auto& v) { return std::abs(v - dataI[0]) <= tolerance; }))
        {
            return dataI[0];
        }
        auto i = (size_t)(std::min_element(dataI.begin(), dataI.end()) - dataI.begin());
        dataI[i] += data0[i];
    }
    THROW_OR_ABORT("least_common_multiple did not terminate");
}

}
