#pragma once
#include <iosfwd>
#include <string>

namespace Mlib {

inline std::istream& rgetline(std::istream& is, std::string& str) {
    auto& res = std::getline(is, str);
    if (!str.empty() && str[str.size() - 1] == '\r')
    {
        str.resize(str.size() - 1);
    }
    return res;
}

}
