#pragma once
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/U32_Regex.hpp>
#include <list>
#include <set>
#include <string>
#include <vector>

#ifndef __clang__
#include <functional>
#include <ranges>
#endif

namespace Mlib {

std::list<std::string> string_to_list(const std::string& str, const Mlib::re::cregex& re, size_t expected_length = SIZE_MAX);
#ifndef WITHOUT_ICU
std::list<std::u8string> string_to_list(const std::u8string& str, const boost::u32regex& re, size_t expected_length = SIZE_MAX);
#endif
std::list<std::string> string_to_list(const std::string& str, size_t expected_length = SIZE_MAX);
std::vector<std::string> string_to_vector(const std::string& str);
std::set<std::string> string_to_set(const std::string& str);
std::set<std::string> string_to_set(const std::string& str, const Mlib::re::cregex& re, size_t expected_length = SIZE_MAX);
template <class TString, class TRegex, class TOperation>
auto string_to_vector(const TString& str, const TRegex& re, const TOperation& op, size_t expected_length = SIZE_MAX) {
    std::list<TString> sresult = string_to_list(str, re, expected_length);
    std::vector<decltype(op(TString{}))> result;
    result.reserve(sresult.size());
    for (const TString& s : sresult) {
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
