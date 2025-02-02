#pragma once

namespace Mlib {

#if defined(__ANDROID__) || defined(__linux__)
void print_stacktrace();
#endif

}
