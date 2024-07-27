#pragma once
#include <string>

namespace Mlib {

enum class RenderingStrategies {
    NONE = 0,
    MESH_ONCE = 1 << 0,
    MESH_SORTED_CONTINUOUSLY = 1 << 1,
    INSTANCES_ONCE = 1 << 2,
    INSTANCES_SORTED_CONTINUOUSLY = 1 << 3,
    OBJECT = 1 << 4
};

inline RenderingStrategies operator | (RenderingStrategies a, RenderingStrategies b) {
    return (RenderingStrategies)((unsigned int)a | (unsigned int)b);
}

inline RenderingStrategies operator & (RenderingStrategies a, RenderingStrategies b) {
    return (RenderingStrategies)((unsigned int)a & (unsigned int)b);
}

inline RenderingStrategies& operator |= (RenderingStrategies& a, RenderingStrategies b) {
    (int&)a |= (int)b;
    return a;
}

inline bool any(RenderingStrategies a) {
    return a != RenderingStrategies::NONE;
}

std::string rendering_strategies_to_string(RenderingStrategies rendering_strategies);
RenderingStrategies rendering_strategy_from_string(const std::string& s);

}
