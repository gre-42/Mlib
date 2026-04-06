#pragma once

namespace Mlib {

void enable_floating_point_exceptions();

class TemporarilyIgnoreFloatingPointExeptions {
public:
    TemporarilyIgnoreFloatingPointExeptions();
    ~TemporarilyIgnoreFloatingPointExeptions();
private:
#ifdef _WIN32
    unsigned int control_word_;
#endif
#ifdef __linux__
#ifndef __ANDROID__
    int fpeflags_;
#endif
#endif
};

}
