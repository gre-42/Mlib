#pragma once
#include "common/shapes.h"
#include <stdexcept>

namespace p2t {

class EdgeException: public std::runtime_error {
public:
    EdgeException(const Point& a, const Point& b, const std::string& what)
      : std::runtime_error{ what }
      , edge{ a, b }
    {}
    Point edge[2];
};

}
