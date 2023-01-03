#pragma once

namespace Mlib {

#if defined(__ANDROID__) || defined(__linux__)
void print_stacktrace(unsigned int max_frames = 100);
#endif

}
