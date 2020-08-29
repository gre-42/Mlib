#pragma once
#include <iosfwd>

namespace Mlib {

static const unsigned int LOG_TIME = 1 << 0;
static const unsigned int LOG_POSITION = 1 << 1;
static const unsigned int LOG_SPEED = 1 << 2;
static const unsigned int LOG_HEALTH = 1 << 3;

class Loggable {
public:
    virtual void log(std::ostream& ostr, unsigned int log_components) const = 0;
};

}
