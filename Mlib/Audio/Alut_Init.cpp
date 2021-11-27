#include "Alut_Init.hpp"
#include <AL/alut.h>
#include <iostream>

using namespace Mlib;

AlutInit::AlutInit() {
    if (!alutInitWithoutContext(nullptr, nullptr)) {
        throw std::runtime_error("Could not initialize alut: " + std::string(alutGetErrorString(alutGetError())));
    }
}

AlutInit::~AlutInit() {
    if (!alutExit()) {
        std::cerr << "Error exiting alut: " << alutGetErrorString(alutGetError()) << std::endl;
    }
}
