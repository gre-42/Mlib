#include "List_Audio_Devices.hpp"
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <AL/al.h>
#include <AL/alc.h>
#include <cstddef>
#include <cstring>
#include <iostream>

using namespace Mlib;

void Mlib::list_audio_devices() {
    std::scoped_lock lock{ al_error_mutex };
    const ALCchar* devices = alcGetString(nullptr, ALC_DEVICE_SPECIFIER);
    if (devices == nullptr) {
        THROW_OR_ABORT("Could not list audio devices");
    }
    const ALCchar* device = devices;
    const ALCchar* next = devices + 1;
    size_t len = 0;

    std::cout << "Audo devices:\n";
    std::cout << "----------\n";
    while (device && *device != '\0' && next && *next != '\0') {
            std::cout << device << '\n';
            len = strlen(device);
            device += (len + 1);
            next += (len + 2);
    }
    std::cout << "----------\n";
}
