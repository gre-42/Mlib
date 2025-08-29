#include "Audio_Distance_Model.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace Mlib;

AudioDistanceModel Mlib::audio_distance_model_from_string(const std::string& s) {
    static const std::map<std::string, AudioDistanceModel> m{
        {"inverse_distance_clamped", AudioDistanceModel::INVERSE_DISTANCE_CLAMPED},
        {"linear_distance_clamped", AudioDistanceModel::LINEAR_DISTANCE_CLAMPED}};
    auto it = m.find(s);
    if (it != m.end()) {
        return it->second;
    }
    THROW_OR_ABORT("Unknown audio distance model: \"" + s + '"');
}
