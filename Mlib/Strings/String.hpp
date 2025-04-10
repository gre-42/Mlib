#pragma once
#include <Mlib/Regex/Regex_Select.hpp>
#include <compare>
#include <list>
#include <set>
#include <string>
#include <vector>

#ifndef __clang__
#include <functional>
#include <ranges>
#endif

namespace Mlib {

std::strong_ordering operator <=> (const std::string& a, const std::string& b);
std::list<std::string> string_to_list(const std::string& str, const Mlib::re::cregex& re, size_t expected_length = SIZE_MAX);
std::list<std::string> string_to_list(const std::string& str, size_t expected_length = SIZE_MAX);
std::vector<std::string> string_to_vector(const std::string& str);
std::set<std::string> string_to_set(const std::string& str);
std::set<std::string> string_to_set(const std::string& str, const Mlib::re::cregex& re, size_t expected_length = SIZE_MAX);
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

template <class TOperation>
auto string_to_vector(const std::string& str, const Mlib::re::cregex& re, const TOperation& op, size_t expected_length = SIZE_MAX) {
    std::list<std::string> sresult = string_to_list(str, re, expected_length);
    std::vector<decltype(op(""))> result;
    result.reserve(sresult.size());
    for (const std::string& s : sresult) {
        result.push_back(op(s));
    }
    return result;
}

template <class TOperation>
auto string_to_vector(const std::string& str, const TOperation& op, size_t expected_length = SIZE_MAX) {
    static const DECLARE_REGEX(re, "\\s+");
    return string_to_vector(str, re, op, expected_length);
}

}
