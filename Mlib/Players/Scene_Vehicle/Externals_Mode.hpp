#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

enum class ExternalsMode: uint32_t {
    PC = 0x9B84A385,
    NPC = 0x2F56F84E,
    NONE = 0x5E78D317
};

ExternalsMode externals_mode_from_string(const std::string& externals_mode);

}
