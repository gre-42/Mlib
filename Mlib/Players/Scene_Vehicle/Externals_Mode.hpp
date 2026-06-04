#pragma once
#include <cstdint>
#include <string>

namespace Mlib {

enum class ExternalsMode: uint32_t {
    PC = 0,
    NPC = 1,
    NONE = 2
};
static const size_t EXTERNALS_MODE_BITS = 2;

ExternalsMode externals_mode_from_string(const std::string& externals_mode);

}
