#include "Stack_Trace.hpp"
#include <Mlib/Os/Os.hpp>

#ifdef __ANDROID__

// From: https://stackoverflow.com/questions/8115192/android-ndk-getting-the-backtrace
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>

struct android_backtrace_state
{
    void **current;
    void **end;
};

_Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context* context,
                                            void* arg)
{
    auto* state = (android_backtrace_state *)arg;
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc)
    {
        if (state->current == state->end)
        {
            return _URC_END_OF_STACK;
        }
        else
        {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

void Mlib::print_stacktrace()
{
    LOGE("Android stack dump");

    static const size_t max_frames = 300;
    void* buffer[max_frames];

    android_backtrace_state state{
        .current = buffer,
        .end = buffer + max_frames};

    _Unwind_Backtrace(android_unwind_callback, &state);

    int count = (int)(state.current - buffer);

    for (int idx = 0; idx < count; idx++)
    {
        const void* addr = buffer[idx];
        const char* symbol = "";

        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname)
        {
            symbol = info.dli_sname;
        }
        int status = 0;
        char *demangled = __cxxabiv1::__cxa_demangle(symbol, nullptr, nullptr, &status);

        LOGE("%03d: 0x%p %s",
             idx,
             addr,
             (nullptr != demangled && 0 == status) ?
             demangled : symbol);

        if (nullptr != demangled)
            free(demangled);
    }

    LOGE("Android stack dump done");
}

#elif __linux__
// stacktrace.h (c) 2008, Timo Bingmann from http://idlebox.net/
// published under the WTFPL v2.0

#include <climits>
#include <stdlib.h>
#include <execinfo.h>
#include <cxxabi.h>

using namespace Mlib;

/** Print a demangled stack backtrace of the caller function to FILE* out. */
void Mlib::print_stacktrace()
{
    lerr() << "stack trace:";

    // storage array for stack trace address data
    static const size_t max_frames = 300;
    void* addrlist[max_frames+1];

    if (max_frames >= INT_MAX) {
        lerr() << "max_frames too large";
        return;
    }
    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, (int)(sizeof(addrlist) / sizeof(void*)));

    if (addrlen == 0) {
        lerr() << "  <empty, possibly corrupt>";
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char* funcname = (char*)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 1; i < addrlen; i++)
    {
        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbollist[i]; *p; ++p)
        {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset
            && begin_name < begin_offset)
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status;
            char* ret = abi::__cxa_demangle(begin_name,
                                            funcname, &funcnamesize, &status);
            if (status == 0) {
                funcname = ret; // use possibly realloc()-ed string
                lerr() << "  " << symbollist[i] << " : " << funcname << '+' << begin_offset;
            }
            else {
                // demangling failed. Output function name as a C function with
                // no arguments.
                lerr() << "  " << symbollist[i] << " : " << begin_name << "()+" << begin_offset;
            }
        }
        else
        {
            // couldn't parse the line? print the whole line.
            lerr() << "  " << symbollist[i];
        }
    }

    free(funcname);
    free(symbollist);
}

#endif
