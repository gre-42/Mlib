#pragma once
#include <string>

namespace Mlib::Sfm {

enum class CostVolumeType {
    MUTLICHANNEL_FLAT,
    MUTLICHANNEL_PYRAMID
};

CostVolumeType cost_volume_type_from_string(const std::string& s);

}
