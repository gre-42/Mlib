#pragma once
#include <Mlib/Audio/OpenAL_al.h>
#include <Mlib/Audio/OpenAL_alc.h>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

namespace Mlib {

extern FastMutex al_error_mutex;
const char* get_al_error_string(ALenum error);

}

#define AL_CHK(f)                                                                                  \
    {                                                                                              \
        std::scoped_lock mutex{Mlib::al_error_mutex};                                              \
        f;                                                                                         \
        {                                                                                          \
            ALCenum error = alGetError();                                                          \
            if (error != AL_NO_ERROR) {                                                            \
                throw std::runtime_error("Error executing " #f ": " +                                        \
                               std::string(get_al_error_string(error)));                            \
            }                                                                                      \
        }                                                                                          \
    }

#define AL_WARN(f)                                                                                 \
    {                                                                                              \
        std::scoped_lock mutex{Mlib::al_error_mutex};                                              \
        f;                                                                                         \
        {                                                                                          \
            ALCenum error = alGetError();                                                          \
            if (error != AL_NO_ERROR) {                                                            \
                lwarn() << "Error executing " #f ": " << get_al_error_string(error);                \
            }                                                                                      \
        }                                                                                          \
    }

#define AL_ABORT(f)                                                                                \
    {                                                                                              \
        std::scoped_lock mutex{Mlib::al_error_mutex};                                              \
        f;                                                                                         \
        {                                                                                          \
            ALCenum error = alGetError();                                                          \
            if (error != AL_NO_ERROR) {                                                            \
                verbose_abort("Error executing " #f ": " +                                         \
                              std::string(get_al_error_string(error)));                             \
            }                                                                                      \
        }                                                                                          \
    }
