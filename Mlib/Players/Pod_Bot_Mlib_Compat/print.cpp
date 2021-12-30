#include <Mlib/Players/Pod_Bot_Mlib_Compat/types.hpp>
#include <iostream>
#include <stdarg.h>

void UTIL_ServerPrint(const char *fmt, ...) {
    // Declare a va_list type variable
    va_list myargs;

    // Initialise the va_list variable with the ... after fmt
    va_start(myargs, fmt);

    // Forward the '...' to vprintf
    if (vprintf(fmt, myargs) < 0) {
        std::cerr << "UTIL_ServerPrint failed" << std::endl;
    }

    // Clean up the va_list
    va_end(myargs);
}

void UTIL_HostPrint (const char *fmt, ...) {
    // Declare a va_list type variable
    va_list myargs;

    // Initialise the va_list variable with the ... after fmt
    va_start(myargs, fmt);

    // Forward the '...' to vprintf
    if (vprintf(fmt, myargs) < 0) {
        std::cerr << "UTIL_HostPrint failed" << std::endl;
    }

    // Clean up the va_list
    va_end(myargs);
}

void UTIL_HudMessage(edict_t*, hudtextparms_t const&, char*) {
    throw std::runtime_error("Not yet implemented");
}

void FakeClientCommand(edict_t* edict, char const* fmt, ...) {
    // Declare a va_list type variable
    va_list myargs;

    // Initialise the va_list variable with the ... after fmt
    va_start(myargs, fmt);

    // Forward the '...' to vprintf
    if (vprintf(fmt, myargs) < 0) {
        std::cerr << "FakeClientCommand failed" << std::endl;
    }

    // Clean up the va_list
    va_end(myargs);
}
