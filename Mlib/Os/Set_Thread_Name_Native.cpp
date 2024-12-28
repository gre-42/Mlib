#include "Set_Thread_Name_Native.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

#ifdef __linux__

// #include <pthread.h>
// #include <cstring>

// void Mlib::set_thread_name_native(const std::string& name) {
//     int rc = pthread_setname_np(pthread_self(), name.c_str());
//     if (rc != 0) {
//         THROW_OR_ABORT(std::string("Could not set thread name: ") + strerror(rc));
//     }
// }

#include <linux/prctl.h>    // PR_* constants
#include <sys/prctl.h>
#include <cerrno>
#include <cstring>          // strerror

void Mlib::set_thread_name_native(const std::string& name) {
    if (prctl(PR_SET_NAME, name.c_str()) != 0) {
        THROW_OR_ABORT(std::string("Could not set thread name: ") + strerror(errno));
    }
}

#else

#include <Windows.h>
#include <processthreadsapi.h>

using namespace Mlib;

void Mlib::set_thread_name_native(const std::string& name) {
    std::wstring wname{ name.begin(), name.end() };
    HRESULT hr = SetThreadDescription(
        GetCurrentThread(),
        wname.c_str()
    );
    if (FAILED(hr)) {
        THROW_OR_ABORT("Could not set thread name \"" + name + '"');
    }
}

#endif
