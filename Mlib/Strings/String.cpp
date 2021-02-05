#include <Mlib/Strings/String.hpp>
#include <Mlib/Regex_Select.hpp>
#include <regex>
#include <stdexcept>

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

std::list<std::string> Mlib::string_to_list(const std::string& str) {
    std::list<std::string> res;
    static const DECLARE_REGEX(re, "\\s+");
    for (auto it = Mlib::re::sregex_token_iterator(str.begin(), str.end(), re, -1, Mlib::re::regex_constants::match_not_null);
        it != Mlib::re::sregex_token_iterator();
        ++it)
    {
        if (!it->str().empty()) {
            res.push_back(*it);
        }
    }
    return res;
}

std::vector<std::string> Mlib::string_to_vector(const std::string& str) {
    auto res = string_to_list(str);
    return std::vector<std::string>{res.begin(), res.end()};
}

std::set<std::string> Mlib::string_to_set(const std::string& str) {
    auto l = string_to_list(str);
    return {l.begin(), l.end()};
}
