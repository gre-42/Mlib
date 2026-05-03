#include "CHK.hpp"

using namespace Mlib;

bool Mlib::CHECK_AL_ERRORS = false;
Mlib::FastMutex Mlib::al_error_mutex;

void Mlib::check_al_errors(CheckAlErrors check) {
    CHECK_AL_ERRORS = (check == CheckAlErrors::ENABLED);
}

const char* Mlib::get_al_error_string(ALenum error) {
    switch (error) {
        case AL_NO_ERROR:          return "No Error";
        case AL_INVALID_NAME:      return "Invalid Name";
        case AL_INVALID_ENUM:      return "Invalid Enum";
        case AL_INVALID_VALUE:     return "Invalid Value";
        case AL_INVALID_OPERATION: return "Invalid Operation";
        case AL_OUT_OF_MEMORY:     return "Out of Memory";
        default:                   return "Unknown Error";
    }
}
