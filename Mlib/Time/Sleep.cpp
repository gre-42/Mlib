#ifdef _MSC_VER

#include "Sleep.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Windows.h>

using namespace Mlib;

// From: https://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
void Mlib::usleep(uint64_t usec) 
{
    LARGE_INTEGER ft{
        .QuadPart = -(10 * integral_cast<int64_t>(usec)) // Convert to 100 nanosecond interval, negative value indicates relative time
    };

    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL);
    if (time == NULL) {
        verbose_abort("CreateWaitableTimer failed");
    }
    if (!SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0)) {
        verbose_abort("SetWaitableTimer failed");
    }
    if (WaitForSingleObject(timer, INFINITE) == WAIT_FAILED) {
        verbose_abort("WaitForSingleObject failed");
    }
    if (!CloseHandle(timer)) {
        verbose_abort("CloseHandle failed");
    }
}

#endif
