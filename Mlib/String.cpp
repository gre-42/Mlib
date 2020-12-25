#include <Mlib/String.hpp>
#include <regex>
#include <stdexcept>

using namespace Mlib;

double Mlib::safe_stod(const std::string& s) {
    std::size_t idx;
    double res;
    try {
        res = std::stod(s, &idx);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("safe_stod: \"" + s + '"');
    }
    if (idx != s.length()) {
        throw std::invalid_argument("safe_stod: \"" + s + '"');
    }
    return res;
}

float Mlib::safe_stof(const std::string& s) {
    std::size_t idx;
    float res;
    try {
        res = std::stof(s, &idx);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("safe_stof: \"" + s + '"');
    }
    if (idx != s.length()) {
        throw std::invalid_argument("safe_stof: \"" + s + '"');
    }
    return res;
}

int Mlib::safe_stoi(const std::string& s) {
    std::size_t idx;
    float res;
    try {
        res = std::stoi(s, &idx);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("safe_stoi: \"" + s + '"');
    }
    if (idx != s.length()) {
        throw std::invalid_argument("safe_stoi: \"" + s + '"');
    }
    return res;
}

size_t Mlib::safe_stoz(const std::string& s) {
    std::size_t idx;
    size_t res;
    try {
        unsigned long ul = std::stoul(s, &idx);
        res = ul;
        if (res != ul) {
            throw std::invalid_argument(s);
        }
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("safe_stoz: \"" + s + '"');
    }
    if (idx != s.length()) {
        throw std::invalid_argument("safe_stoz: \"" + s + '"');
    }
    return res;
}

bool Mlib::safe_stob(const std::string& s) {
    if (s == "0") {
        return false;
    }
    if (s == "1") {
        return true;
    }
    throw std::runtime_error("Could not convert \"" + s + "\" to bool");
}

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
    static const std::regex re{"\\s+"};
    for (auto it = std::sregex_token_iterator(str.begin(), str.end(), re, -1, std::regex_constants::match_not_null);
        it != std::sregex_token_iterator();
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

std::string Mlib::join(const std::string& delimiter, const std::list<std::string>& lst) {
    std::string res;
    int i = 0;
    for (const std::string& s : lst) {
        res += (i++ == 0)
            ? s
            : delimiter + s;
    }
    return res;
}
