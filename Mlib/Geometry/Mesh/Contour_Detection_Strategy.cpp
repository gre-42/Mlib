
#include "Contour_Detection_Strategy.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

ContourDetectionStrategy Mlib::contour_detection_strategy_from_string(const std::string& s) {
    static const std::map<std::string, ContourDetectionStrategy> m{
        { "node_neighbor", ContourDetectionStrategy::NODE_NEIGHBOR },
        { "edge_neighbor", ContourDetectionStrategy::EDGE_NEIGHBOR },
        { "triangle", ContourDetectionStrategy::TRIANGLE },
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown contour detection strategy: \"" + s + '"');
    }
    return it->second;
}
