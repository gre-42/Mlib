#include "Get_Thread_Name.hpp"
#include <stdexcept>

#ifdef __EMSCRIPTEN__

#include <Mlib/Os/Set_Thread_Name_Native.hpp>

std::string Mlib::get_thread_name() {
    return get_thread_name_native();
}

#elifdef  __linux__

#include <linux/prctl.h>    // PR_* constants
#include <sys/prctl.h>
#include <cerrno>
#include <cstring>          // strerror

std::string Mlib::get_thread_name() {
    char buf[16];
    if (prctl(PR_GET_NAME, buf) != 0) {
        throw std::runtime_error(std::string("Could not get thread name: ") + strerror(errno));
    }
    return { buf };
}

#else

#include <Windows.h>
#include <processthreadsapi.h>

using namespace Mlib;

std::string Mlib::get_thread_name() {
    PWSTR data;
    auto hr = GetThreadDescription(GetCurrentThread(), &data);
    if (FAILED(hr)) {
        throw std::runtime_error("Could not get thread name");
    }
    std::string result(wcslen(data), '?');
    for (size_t i = 0; i < result.size(); ++i) {
        auto c = (char)data[i];
        if (c == data[i]) {
            result[i] = c;
        }
    }
    LocalFree(data);
    return result;
}

#endif
