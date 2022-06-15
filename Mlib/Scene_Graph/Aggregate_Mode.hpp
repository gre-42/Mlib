#pragma once
#include <string>

namespace Mlib {

enum class AggregateMode {
    OFF = 1 << 0,
    ONCE = 1 << 1,
    SORTED_CONTINUOUSLY = 1 << 2,
    INSTANCES_ONCE = 1 << 3,
    INSTANCES_SORTED_CONTINUOUSLY = 1 << 4
};

inline unsigned int operator | (AggregateMode a, AggregateMode b) {
    return (unsigned int)a | (unsigned int)b;
}

inline unsigned int operator & (AggregateMode a, unsigned int b) {
    return (unsigned int)a & b;
}

AggregateMode aggregate_mode_from_string(const std::string& str);
std::string aggregate_mode_to_string(AggregateMode aggregate_mode);

}
