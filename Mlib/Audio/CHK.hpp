#pragma once
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <AL/alut.h>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

namespace Mlib {

extern AtomicMutex al_error_mutex;

}

#define AL_CHK(f)                                                                                  \
    {                                                                                              \
        std::scoped_lock mutex{Mlib::al_error_mutex};                                              \
        f;                                                                                         \
        {                                                                                          \
            ALCenum error = alGetError();                                                          \
            if (error != AL_NO_ERROR) {                                                            \
                THROW_OR_ABORT("Error executing " #f ": " +                                        \
                               std::string(alutGetErrorString(error)));                            \
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
                lwarn() << "Error executing " #f ": " << alutGetErrorString(error);                \
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
                              std::string(alutGetErrorString(error)));                             \
            }                                                                                      \
        }                                                                                          \
    }
