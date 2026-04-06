#pragma once
#include <string>

namespace Mlib {

enum class BloomMode {
    STANDARD,
    SKY
};

BloomMode bloom_mode_from_string(const std::string& s);

}
