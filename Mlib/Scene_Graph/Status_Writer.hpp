#pragma once
#include <iosfwd>

namespace Mlib {

static const unsigned int STATUS_TIME = 1 << 0;
static const unsigned int STATUS_POSITION = 1 << 1;
static const unsigned int STATUS_SPEED = 1 << 2;
static const unsigned int STATUS_HEALTH = 1 << 3;
static const unsigned int STATUS_ACCELERATION = 1 << 4;
static const unsigned int STATUS_DIAMETER = 1 << 5;
static const unsigned int STATUS_DIAMETER2 = 1 << 6;
static const unsigned int STATUS_ENERGY = 1 << 7;

class StatusWriter {
public:
    virtual void write_status(std::ostream& ostr, unsigned int status_components) const = 0;
};

}
