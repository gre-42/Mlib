#pragma once

namespace Mlib {

#if defined(__ANDROID__) || defined(__linux__)
void print_stack_trace_on_abort();
#endif

}
