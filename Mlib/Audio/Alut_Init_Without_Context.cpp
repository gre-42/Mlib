
#include "Alut_Init_Without_Context.hpp"
#include <Mlib/Os/Os.hpp>
#include <AL/alut.h>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

AlutInitWithoutContext::AlutInitWithoutContext() {
    if (!alutInitWithoutContext(nullptr, nullptr)) {
        throw std::runtime_error("Could not initialize alut: " + std::string(alutGetErrorString(alutGetError())));
    }
}

AlutInitWithoutContext::~AlutInitWithoutContext() {
    if (!alutExit()) {
        verbose_abort("Error exiting alut: " + std::string(alutGetErrorString(alutGetError())));
    }
}
