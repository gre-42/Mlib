#include "List_Audio_Devices.hpp"
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Audio/OpenAL_al.h>
#include <Mlib/Audio/OpenAL_alc.h>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstddef>
#include <cstring>
#include <mutex>

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

    linfo() << "Audio devices:";
    linfo() << "----------";
    while (device && *device != '\0' && next && *next != '\0') {
        linfo() << device;
        len = strlen(device);
        device += (len + 1);
        next += (len + 2);
    }
    linfo() << "----------";
}
