
#include "Audio_Distance_Model.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

AudioDistanceModel Mlib::audio_distance_model_from_string(const std::string& s) {
    static const std::map<std::string, AudioDistanceModel> m{
        {"inverse_distance_clamped", AudioDistanceModel::INVERSE_DISTANCE_CLAMPED},
        {"linear_distance_clamped", AudioDistanceModel::LINEAR_DISTANCE_CLAMPED}};
    auto it = m.find(s);
    if (it != m.end()) {
        return it->second;
    }
    throw std::runtime_error("Unknown audio distance model: \"" + s + '"');
}
