#pragma once
#include <string>

namespace Mlib {

enum class ContourDetectionStrategy {
    NODE_NEIGHBOR,
    EDGE_NEIGHBOR,
    TRIANGLE
};

ContourDetectionStrategy contour_detection_strategy_from_string(const std::string& s);

}
