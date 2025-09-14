#pragma once
#include <string>

namespace Mlib {

enum class FilterType {
    NONE,
    FILTERED
};

FilterType filter_type_from_string(const std::string& s);

}
