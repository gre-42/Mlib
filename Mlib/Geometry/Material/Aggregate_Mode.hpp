#pragma once
#include <string>

namespace Mlib {

enum class AggregateMode {
    NONE = 0,
    ONCE = 1 << 0,
    SORTED_CONTINUOUSLY = 1 << 1,
    INSTANCES_ONCE = 1 << 2,
    INSTANCES_SORTED_CONTINUOUSLY = 1 << 3,
    OBJECT_MASK = ONCE | SORTED_CONTINUOUSLY,
    INSTANCES_MASK = INSTANCES_ONCE | INSTANCES_SORTED_CONTINUOUSLY
};

inline AggregateMode operator | (AggregateMode a, AggregateMode b) {
    return (AggregateMode)((unsigned int)a | (unsigned int)b);
}

inline AggregateMode operator & (AggregateMode a, AggregateMode b) {
    return (AggregateMode)((unsigned int)a & (unsigned int)b);
}

inline AggregateMode operator ~ (AggregateMode a) {
    return (AggregateMode)(~(int)a);
}

inline bool any(AggregateMode a) {
    return a != AggregateMode::NONE;
}

AggregateMode aggregate_mode_from_string(const std::string& str);
std::string aggregate_mode_to_string(AggregateMode aggregate_mode);

}
