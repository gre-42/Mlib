#pragma once
#include <AL/alut.h>
#include <iostream>
#include <stdexcept>
#include <string>

#define AL_CHK(f) \
    f; \
    { \
        ALCenum error = alGetError(); \
        if (error != AL_NO_ERROR) { \
            throw std::runtime_error("Error executing " #f ": " + std::string(alutGetErrorString(error))); \
        } \
    }

#define AL_WARN(f) \
    f; \
    { \
        ALCenum error = alGetError(); \
        if (error != AL_NO_ERROR) { \
            std::cerr << "Error executing " #f ": " << alutGetErrorString(error); \
        } \
    }
