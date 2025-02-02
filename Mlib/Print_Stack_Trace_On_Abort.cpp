#include "Print_Stack_Trace_On_Abort.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Stack_Trace.hpp>
#include <csignal>

using namespace Mlib;

#if defined(__ANDROID__) || defined(__linux__)

extern "C" void abort_handler(int signal_number)
{
    lerr() << "Abort called";
    print_stacktrace();
    lerr() << "Calling exit(134)";
    std::exit(134);
}

void Mlib::print_stack_trace_on_abort() {
    signal(SIGABRT, &abort_handler);
}

#endif
