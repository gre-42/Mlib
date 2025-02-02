#include "Rendering_Strategies.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

static std::string rendering_strategy_to_string(RenderingStrategies rendering_strategy) {
    switch (rendering_strategy) {
    case RenderingStrategies::NONE:
        return "none";
    case RenderingStrategies::MESH_ONCE:
        return "mesh_once";
    case RenderingStrategies::MESH_SORTED_CONTINUOUSLY:
        return "mesh_sorted_continuously";
    case RenderingStrategies::INSTANCES_ONCE:
        return "instances_once";
    case RenderingStrategies::INSTANCES_SORTED_CONTINUOUSLY:
        return "instances_sorted_continuously";
    case RenderingStrategies::OBJECT:
        return "object";
    }
    THROW_OR_ABORT("Unknown rendering strategy: " + std::to_string((int)rendering_strategy));
}

std::string Mlib::rendering_strategies_to_string(RenderingStrategies rendering_strategies) {
    std::string result = "";
    using UT = std::underlying_type<RenderingStrategies>::type;
    for (UT ucandidate = 1; ucandidate != 0; ucandidate <<= 1) {
        auto candidate = (RenderingStrategies)ucandidate;
        if (any(rendering_strategies & candidate)) {
            if (result.empty()) {
                result += '|' + rendering_strategy_to_string(candidate);
            } else {
                result = rendering_strategy_to_string(candidate);
            }
        }
    }
    if (result.empty()) {
        return rendering_strategy_to_string(RenderingStrategies::NONE);
    }
    return result;
}

RenderingStrategies Mlib::rendering_strategy_from_string(const std::string& s) {
    static const std::map<std::string, RenderingStrategies> m{
        {"none", RenderingStrategies::NONE},
        {"mesh_once", RenderingStrategies::MESH_ONCE},
        {"mesh_sorted_continuously", RenderingStrategies::MESH_SORTED_CONTINUOUSLY},
        {"instances_once", RenderingStrategies::INSTANCES_ONCE},
        {"instances_sorted_continuously", RenderingStrategies::INSTANCES_SORTED_CONTINUOUSLY},
        {"object", RenderingStrategies::OBJECT}};
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown rendering strategy: \"" + s + '"');
    }
    return it->second;
}
