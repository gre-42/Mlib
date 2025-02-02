#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>

using namespace Mlib;

std::strong_ordering Mlib::operator <=> (const std::string& a, const std::string& b) {
    int i = a.compare(b);
    if (i < 0) {
        return std::strong_ordering::less;
    }
    if (i == 0) {
        return std::strong_ordering::equal;
    }
    return std::strong_ordering::greater;
}

std::list<std::string> Mlib::string_to_list(const std::string& str, const Mlib::regex& re, size_t expected_length) {
    std::list<std::string> res;
    if (str.empty()) {
        return res;
    }
    for (auto it = Mlib::re::sregex_token_iterator(str.begin(), str.end(), re, -1, Mlib::re::regex_constants::match_not_null);
        it != Mlib::re::sregex_token_iterator();
        ++it)
    {
        res.push_back(*it);
    }
    if ((expected_length != SIZE_MAX) && (res.size() != expected_length)) {
        THROW_OR_ABORT("Expected " + std::to_string(expected_length) + " elements, but got " + std::to_string(res.size()));
    }
    return res;
}

std::list<std::string> Mlib::string_to_list(const std::string& str, size_t expected_length) {
    static const DECLARE_REGEX(re, "\\s+");
    return string_to_list(str, re, expected_length);
}

std::vector<std::string> Mlib::string_to_vector(const std::string& str) {
    auto res = string_to_list(str);
    return std::vector<std::string>{res.begin(), res.end()};
}

std::set<std::string> Mlib::string_to_set(const std::string& str) {
    auto l = string_to_list(str);
    return {l.begin(), l.end()};
}

std::set<std::string> Mlib::string_to_set(const std::string& str, const Mlib::regex& re, size_t expected_length) {
    auto l = string_to_list(str, re, expected_length);
    return {l.begin(), l.end()};
}
