#pragma once
#include <list>
#include <set>
#include <string>
#include <vector>

#ifndef __clang__
#include <functional>
#include <ranges>
#endif

namespace Mlib {

#ifdef __clang__
inline const std::string& identity_(const std::string& v) {
    return v;
}
template <class TContainer, class TOperation = decltype(identity_)>
std::string join(
    const std::string& delimiter,
    const TContainer& lst,
    const TOperation& op = identity_)
#else
template <class TContainer, class TOperation = std::identity>
requires std::ranges::range<TContainer>
std::string join(const std::string& delimiter, const TContainer& lst, const TOperation& op = {})
#endif
{
    std::string res;
    int i = 0;
    for (const auto& s : lst) {
        res += (i++ == 0)
            ? op(s)
            : delimiter + op(s);
    }
    return res;
}

}
