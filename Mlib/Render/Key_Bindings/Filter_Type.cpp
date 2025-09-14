#include "Filter_Type.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

FilterType Mlib::filter_type_from_string(const std::string& s) {
    static const std::map<std::string, FilterType> m{
        { "none", FilterType::NONE },
        { "filtered", FilterType::FILTERED }
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown filter type: \"" + s + '"');
    }
    return it->second;
}
