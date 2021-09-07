#include "Cost_Volume_Type.hpp"
#include <stdexcept>

using namespace Mlib::Sfm;

CostVolumeType Mlib::Sfm::cost_volume_type_from_string(const std::string& s) {
    if (s == "multichannel_flat") {
        return CostVolumeType::MUTLICHANNEL_FLAT;
    } else if (s == "multichannel_pyramid") {
        return CostVolumeType::MUTLICHANNEL_PYRAMID;
    } else {
        throw std::runtime_error("Unknown cost volume type: \"" + s + '"');
    }
}
