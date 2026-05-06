#include "Filter_Type.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

FilterType Mlib::filter_type_from_string(const std::string& s) {
    static const std::map<std::string, FilterType> m{
        { "none", FilterType::NONE },
        { "filtered", FilterType::FILTERED }
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown filter type: \"" + s + '"');
    }
    return it->second;
}
