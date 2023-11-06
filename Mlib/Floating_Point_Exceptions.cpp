#include "Floating_Point_Exceptions.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifdef _WIN32

#include <float.h>
#include <stdexcept>
#include <iostream>

using namespace Mlib;

// #ifdef _MSC_VER
// #pragma float_control(except, on)
// #endif

void Mlib::enable_floating_point_exceptions() {
    unsigned int control_word;
    {
        errno_t err = _controlfp_s(&control_word, 0, 0);
        if (err != 0) {
            THROW_OR_ABORT("Could not get float point control word");
        }
    }
    // From: https://stackoverflow.com/questions/4282217/visual-c-weird-behavior-after-enabling-floating-point-exceptions-compiler-b
    control_word &= ~(unsigned int)_EM_INVALID;
    {
        unsigned int control_word2 = 0;
        errno_t err = _controlfp_s(&control_word2, control_word, _MCW_EM);
        if (err != 0) {
            THROW_OR_ABORT("Could not set float point control word");
        }
    }
}

TemporarilyIgnoreFloatingPointExeptions::TemporarilyIgnoreFloatingPointExeptions()
{
    {
        errno_t err = _controlfp_s(&control_word_, 0, 0);
        if (err != 0) {
            THROW_OR_ABORT("Could not read control word");
        }
    }
    {
        unsigned int control_word = 0;
        errno_t err = _controlfp_s(&control_word, _CW_DEFAULT, _MCW_EM);
        if (err != 0) {
            THROW_OR_ABORT("Could not restore default control word");
        }
    }
}

TemporarilyIgnoreFloatingPointExeptions::~TemporarilyIgnoreFloatingPointExeptions() {
    unsigned int control_word2 = 0;
    errno_t err = _controlfp_s(&control_word2, control_word_, _MCW_EM);
    if (err != 0) {
        verbose_abort("Could not restore previous control word");
    }
}
#endif

#ifdef __linux__

#include <Mlib/Floating_Point_Exceptions.hpp>
#include <cfenv>

using namespace Mlib;

#ifdef __ANDROID__

void Mlib::enable_floating_point_exceptions()
{}

TemporarilyIgnoreFloatingPointExeptions::TemporarilyIgnoreFloatingPointExeptions() = default;

TemporarilyIgnoreFloatingPointExeptions::~TemporarilyIgnoreFloatingPointExeptions() = default;

#else

void Mlib::enable_floating_point_exceptions() {
    if (feenableexcept(FE_INVALID) == -1) {
        THROW_OR_ABORT("Could not enable floating-point exceptions");
    }
}

TemporarilyIgnoreFloatingPointExeptions::TemporarilyIgnoreFloatingPointExeptions()
: fpeflags_{fedisableexcept(FE_ALL_EXCEPT)}
{
    if (fpeflags_ == -1) {
        THROW_OR_ABORT("Could not disable floating-point exceptions");
    }
}

TemporarilyIgnoreFloatingPointExeptions::~TemporarilyIgnoreFloatingPointExeptions() {
    if (feenableexcept(fpeflags_) == -1) {
        verbose_abort("Could not re-enable floating-point exceptions");
    }
}

#endif
#endif
