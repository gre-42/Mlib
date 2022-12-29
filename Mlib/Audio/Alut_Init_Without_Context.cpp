#include "Alut_Init_Without_Context.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <AL/alut.h>
#include <iostream>

using namespace Mlib;

AlutInitWithoutContext::AlutInitWithoutContext() {
    if (!alutInitWithoutContext(nullptr, nullptr)) {
        THROW_OR_ABORT("Could not initialize alut: " + std::string(alutGetErrorString(alutGetError())));
    }
}

AlutInitWithoutContext::~AlutInitWithoutContext() {
    if (!alutExit()) {
        verbose_abort("Error exiting alut: " + std::string(alutGetErrorString(alutGetError())));
    }
}
