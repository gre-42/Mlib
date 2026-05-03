#include "AEmscripten_Result_To_String.hpp"

using namespace Mlib;

const char* Mlib::emscripten_result_to_str(EMSCRIPTEN_RESULT result) {
    switch (result) {
        case EMSCRIPTEN_RESULT_SUCCESS:             return "Success";
        case EMSCRIPTEN_RESULT_DEFERRED:            return "Deferred";
        case EMSCRIPTEN_RESULT_NOT_SUPPORTED:       return "Not Supported";
        case EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED: return "Failed (Not Deferred)";
        case EMSCRIPTEN_RESULT_INVALID_TARGET:      return "Invalid Target";
        case EMSCRIPTEN_RESULT_UNKNOWN_TARGET:      return "Unknown Target";
        case EMSCRIPTEN_RESULT_INVALID_PARAM:       return "Invalid Parameter";
        case EMSCRIPTEN_RESULT_FAILED:              return "Failed";
        case EMSCRIPTEN_RESULT_NO_DATA:             return "No Data";
        case EMSCRIPTEN_RESULT_TIMED_OUT:           return "Timed Out";
        default:                                    return "Unknown Error";
    }
}
