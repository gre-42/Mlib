#include "Set_Thread_Name.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

#ifdef __GNUC__

#include <pthread.h>
#include <cstring>

void Mlib::set_thread_name(const std::string& name) {
    int rc = pthread_setname_np(pthread_self(), name.c_str());
    if (rc != 0) {
        THROW_OR_ABORT(std::string("Could not set thread name: ") + strerror(rc));
    }
}

#else

#include <windows.h>
#include <processthreadsapi.h>

using namespace Mlib;

void Mlib::set_thread_name(const std::string& name) {
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
