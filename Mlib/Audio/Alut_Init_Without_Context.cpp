#include "Alut_Init_Without_Context.hpp"
#include <AL/alut.h>
#include <iostream>

using namespace Mlib;

AlutInitWithoutContext::AlutInitWithoutContext() {
    if (!alutInitWithoutContext(nullptr, nullptr)) {
        throw std::runtime_error("Could not initialize alut: " + std::string(alutGetErrorString(alutGetError())));
    }
}

AlutInitWithoutContext::~AlutInitWithoutContext() {
    if (!alutExit()) {
        std::cerr << "Error exiting alut: " << alutGetErrorString(alutGetError()) << std::endl;
    }
}
