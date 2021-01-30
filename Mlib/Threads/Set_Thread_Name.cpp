#include "Set_Thread_Name.hpp"

#ifdef _WIN32

#include <windows.h>
#include <processthreadsapi.h>
#include <stdexcept>

using namespace Mlib;

void Mlib::set_thread_name(const std::string& name) {
    std::wstring wname{ name.begin(), name.end() };
    HRESULT hr = SetThreadDescription(
        GetCurrentThread(),
        wname.c_str()
    );
    if (FAILED(hr)) {
        throw std::runtime_error("Could not set thread name \"" + name + '"');
    }
}
#endif
