#pragma once
#include <Mlib/Regex_Select.hpp>
#include <compare>
#include <functional>
#include <list>
#include <ranges>
#include <set>
#include <string>
#include <vector>

namespace Mlib {

std::strong_ordering operator <=> (const std::string& a, const std::string& b);
std::list<std::string> string_to_list(const std::string& str, const Mlib::regex& re, size_t expected_length = SIZE_MAX);
std::list<std::string> string_to_list(const std::string& str, size_t expected_length = SIZE_MAX);
std::vector<std::string> string_to_vector(const std::string& str);
std::set<std::string> string_to_set(const std::string& str);
template <class TContainer, class TOperation = std::identity>
requires std::ranges::range<TContainer>
std::string join(const std::string& delimiter, const TContainer& lst, const TOperation& op = {}) {
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
auto string_to_vector(const std::string& str, const TOperation& op, size_t expected_length = SIZE_MAX) {
    std::list<std::string> sresult = string_to_list(str, expected_length);
    std::vector<decltype(op(""))> result;
    result.reserve(sresult.size());
    for (const std::string& s : sresult) {
        result.push_back(op(s));
    }
    return result;
}

}
