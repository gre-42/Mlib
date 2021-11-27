#pragma once
#include <AL/alut.h>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>

namespace Mlib {

extern std::mutex al_error_mutex;

}

#define AL_CHK(f) \
    { \
        std::lock_guard mutex{ Mlib::al_error_mutex }; \
        f; \
        { \
            ALCenum error = alGetError(); \
            if (error != AL_NO_ERROR) { \
                throw std::runtime_error("Error executing " #f ": " + std::string(alutGetErrorString(error))); \
            } \
        } \
    }

#define AL_WARN(f) \
    { \
        std::lock_guard mutex{ Mlib::al_error_mutex }; \
        f; \
        { \
            ALCenum error = alGetError(); \
            if (error != AL_NO_ERROR) { \
                std::cerr << "Error executing " #f ": " << alutGetErrorString(error); \
            } \
        } \
    }
