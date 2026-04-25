#pragma once
#include <string>

namespace Mlib {

void set_thread_name_native(const std::string& name);

#ifdef __EMSCRIPTEN__
std::string get_thread_name_native();
#endif

}
