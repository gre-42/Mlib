#pragma once
#include "common/shapes.h"
#include <stdexcept>

namespace p2t {

class PointException: public std::runtime_error {
public:
    PointException(const Point& point, const std::string& what)
      : std::runtime_error{ what }
      , point{ point }
    {}
    Point point;
};

}
