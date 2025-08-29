#pragma once
#include <string>

namespace Mlib {

enum class AudioDistanceModel {
    INVERSE_DISTANCE_CLAMPED,
    LINEAR_DISTANCE_CLAMPED
};

AudioDistanceModel audio_distance_model_from_string(const std::string& s);

}
