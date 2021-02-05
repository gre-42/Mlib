#pragma once
#include <compare>
#include <functional>
#include <list>
#include <set>
#include <string>
#include <vector>

namespace Mlib {

double safe_stod(const std::string& s);
float safe_stof(const std::string& s);
int safe_stoi(const std::string& s);
unsigned int safe_stou(const std::string& s);
size_t safe_stoz(const std::string& s);
bool safe_stob(const std::string& s);
std::strong_ordering operator <=> (const std::string& a, const std::string& b);
std::list<std::string> string_to_list(const std::string& str);
std::vector<std::string> string_to_vector(const std::string& str);
std::set<std::string> string_to_set(const std::string& str);
template <class TContainer, class TOperation = std::identity>
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
auto string_to_vector(const std::string& str, const TOperation& op) {
    std::list<std::string> sresult = string_to_list(str);
    std::vector<decltype(op(""))> result;
    result.reserve(sresult.size());
    for (const std::string& s : sresult) {
        result.push_back(op(s));
    }
    return result;
}

}
