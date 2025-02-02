#pragma once
#include <string>

namespace Mlib {

enum class ExternalsMode {
    PC,
    NPC,
    NONE
};

ExternalsMode externals_mode_from_string(const std::string& externals_mode);

}
