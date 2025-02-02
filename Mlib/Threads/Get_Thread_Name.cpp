#include "Get_Thread_Name.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

#ifdef __linux__

#include <linux/prctl.h>    // PR_* constants
#include <sys/prctl.h>
#include <cerrno>
#include <cstring>          // strerror

std::string Mlib::get_thread_name() {
    char buf[16];
    if (prctl(PR_GET_NAME, buf) != 0) {
        THROW_OR_ABORT(std::string("Could not get thread name: ") + strerror(errno));
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
        THROW_OR_ABORT("Could not get thread name");
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
